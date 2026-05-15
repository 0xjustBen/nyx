#include "core/cert.hpp"
#include "core/riot_paths.hpp"

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

namespace nyx {
namespace {

struct EvpPkeyDel { void operator()(EVP_PKEY *p) const { EVP_PKEY_free(p); } };
struct X509Del    { void operator()(X509 *p)    const { X509_free(p); } };
struct X509ExtDel { void operator()(X509_EXTENSION *p) const { X509_EXTENSION_free(p); } };
struct BioDel     { void operator()(BIO *p)     const { BIO_free_all(p); } };
struct AsnIntDel  { void operator()(ASN1_INTEGER *p) const { ASN1_INTEGER_free(p); } };
struct BnDel      { void operator()(BIGNUM *p)  const { BN_free(p); } };

using EvpPkey = std::unique_ptr<EVP_PKEY, EvpPkeyDel>;
using X509Up  = std::unique_ptr<X509, X509Del>;
using BioUp   = std::unique_ptr<BIO, BioDel>;
using BnUp    = std::unique_ptr<BIGNUM, BnDel>;

EvpPkey genEcKey()
{
    EVP_PKEY *raw = EVP_EC_gen("P-256");
    return EvpPkey(raw);
}

bool setSerialRandom(X509 *crt)
{
    BnUp bn(BN_new());
    if (!bn || !BN_rand(bn.get(), 159, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY)) return false;
    std::unique_ptr<ASN1_INTEGER, AsnIntDel> ai(ASN1_INTEGER_new());
    if (!BN_to_ASN1_INTEGER(bn.get(), ai.get())) return false;
    return X509_set_serialNumber(crt, ai.get()) == 1;
}

void addExt(X509 *crt, X509 *issuer, int nid, const char *value)
{
    X509V3_CTX ctx;
    X509V3_set_ctx_nodb(&ctx);
    X509V3_set_ctx(&ctx, issuer, crt, nullptr, nullptr, 0);
    X509_EXTENSION *ex = X509V3_EXT_conf_nid(nullptr, &ctx, nid, value);
    if (!ex) return;
    X509_add_ext(crt, ex, -1);
    X509_EXTENSION_free(ex);
}

bool setSubject(X509 *crt, const char *cn)
{
    X509_NAME *n = X509_NAME_new();
    X509_NAME_add_entry_by_txt(n, "O",  MBSTRING_ASC, (const unsigned char *)"Nyx", -1, -1, 0);
    X509_NAME_add_entry_by_txt(n, "CN", MBSTRING_ASC, (const unsigned char *)cn, -1, -1, 0);
    int rc = X509_set_subject_name(crt, n);
    X509_NAME_free(n);
    return rc == 1;
}

std::vector<unsigned char> pemDumpCert(X509 *crt)
{
    BioUp bio(BIO_new(BIO_s_mem()));
    if (!PEM_write_bio_X509(bio.get(), crt)) return {};
    BUF_MEM *bm = nullptr;
    BIO_get_mem_ptr(bio.get(), &bm);
    return std::vector<unsigned char>(bm->data, bm->data + bm->length);
}

std::vector<unsigned char> pemDumpKey(EVP_PKEY *k)
{
    BioUp bio(BIO_new(BIO_s_mem()));
    if (!PEM_write_bio_PrivateKey(bio.get(), k, nullptr, nullptr, 0, nullptr, nullptr)) return {};
    BUF_MEM *bm = nullptr;
    BIO_get_mem_ptr(bio.get(), &bm);
    return std::vector<unsigned char>(bm->data, bm->data + bm->length);
}

bool writeFile(const fs::path &p, const std::vector<unsigned char> &bytes)
{
    std::ofstream o(p, std::ios::binary | std::ios::trunc);
    if (!o) return false;
    o.write(reinterpret_cast<const char *>(bytes.data()), (std::streamsize)bytes.size());
    return o.good();
}

bool readFile(const fs::path &p, std::vector<unsigned char> &out)
{
    std::ifstream in(p, std::ios::binary);
    if (!in) return false;
    std::stringstream ss; ss << in.rdbuf();
    auto s = ss.str();
    out.assign(s.begin(), s.end());
    return true;
}

} // namespace

CertBundle Cert::generate()
{
    CertBundle out;

    EvpPkey caKey = genEcKey();
    EvpPkey leafKey = genEcKey();
    if (!caKey || !leafKey) return out;

    // CA cert
    X509Up ca(X509_new());
    X509_set_version(ca.get(), 2);
    setSerialRandom(ca.get());
    X509_gmtime_adj(X509_getm_notBefore(ca.get()), 0);
    X509_gmtime_adj(X509_getm_notAfter(ca.get()), 60L * 60 * 24 * 365 * 10);
    setSubject(ca.get(), "Nyx Local Root CA");
    X509_set_issuer_name(ca.get(), X509_get_subject_name(ca.get()));
    X509_set_pubkey(ca.get(), caKey.get());
    addExt(ca.get(), ca.get(), NID_basic_constraints,     "critical,CA:TRUE");
    addExt(ca.get(), ca.get(), NID_key_usage,             "critical,keyCertSign,cRLSign");
    addExt(ca.get(), ca.get(), NID_subject_key_identifier, "hash");
    if (!X509_sign(ca.get(), caKey.get(), EVP_sha256())) return out;

    // Leaf cert with SANs for all known chat hosts.
    X509Up leaf(X509_new());
    X509_set_version(leaf.get(), 2);
    setSerialRandom(leaf.get());
    X509_gmtime_adj(X509_getm_notBefore(leaf.get()), 0);
    X509_gmtime_adj(X509_getm_notAfter(leaf.get()), 60L * 60 * 24 * 365);
    setSubject(leaf.get(), "chat.lol.riotgames.com");
    X509_set_issuer_name(leaf.get(), X509_get_subject_name(ca.get()));
    X509_set_pubkey(leaf.get(), leafKey.get());

    std::string san = "DNS:localhost,IP:127.0.0.1";
    for (const auto &h : RiotPaths::chatHosts()) san += ",DNS:" + h;
    addExt(leaf.get(), ca.get(), NID_basic_constraints,     "critical,CA:FALSE");
    addExt(leaf.get(), ca.get(), NID_key_usage,             "critical,digitalSignature,keyEncipherment");
    addExt(leaf.get(), ca.get(), NID_ext_key_usage,         "serverAuth");
    addExt(leaf.get(), ca.get(), NID_subject_alt_name,      san.c_str());
    addExt(leaf.get(), ca.get(), NID_subject_key_identifier,"hash");
    addExt(leaf.get(), ca.get(), NID_authority_key_identifier, "keyid:always");
    if (!X509_sign(leaf.get(), caKey.get(), EVP_sha256())) return out;

    out.caPem      = pemDumpCert(ca.get());
    out.caKeyPem   = pemDumpKey(caKey.get());
    out.leafPem    = pemDumpCert(leaf.get());
    out.leafKeyPem = pemDumpKey(leafKey.get());
    return out;
}

bool Cert::save(const CertBundle &b, const std::string &dir)
{
#ifdef _WIN32
    fs::path d = fs::u8path(dir);
#else
    fs::path d(dir);
#endif
    std::error_code ec;
    fs::create_directories(d, ec);
    if (!writeFile(d / "ca.pem",       b.caPem))      return false;
    if (!writeFile(d / "ca.key",       b.caKeyPem))   return false;
    if (!writeFile(d / "leaf.pem",     b.leafPem))    return false;
    if (!writeFile(d / "leaf.key",     b.leafKeyPem)) return false;
#ifndef _WIN32
    fs::permissions(d / "ca.key",   fs::perms::owner_read | fs::perms::owner_write, ec);
    fs::permissions(d / "leaf.key", fs::perms::owner_read | fs::perms::owner_write, ec);
#endif
    return true;
}

bool Cert::load(CertBundle &out, const std::string &dir)
{
#ifdef _WIN32
    fs::path d = fs::u8path(dir);
#else
    fs::path d(dir);
#endif
    if (!readFile(d / "ca.pem",   out.caPem))      return false;
    if (!readFile(d / "ca.key",   out.caKeyPem))   return false;
    if (!readFile(d / "leaf.pem", out.leafPem))    return false;
    if (!readFile(d / "leaf.key", out.leafKeyPem)) return false;
    return true;
}

} // namespace nyx
