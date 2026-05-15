#include "core/cert.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;
#endif

namespace nyx {

#if defined(_WIN32)
namespace {

std::wstring certDir()
{
    PWSTR roaming = nullptr;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming);
    std::wstring dir = roaming ? roaming : L".";
    if (roaming) CoTaskMemFree(roaming);
    dir += L"\\Nyx";
    fs::create_directories(dir);
    return dir;
}

std::vector<unsigned char> pemToDer(const fs::path &pem)
{
    std::ifstream in(pem, std::ios::binary);
    std::stringstream ss; ss << in.rdbuf();
    std::string s = ss.str();
    auto b = s.find("-----BEGIN CERTIFICATE-----");
    auto e = s.find("-----END CERTIFICATE-----");
    if (b == std::string::npos || e == std::string::npos) return {};
    b += sizeof("-----BEGIN CERTIFICATE-----") - 1;
    std::string body = s.substr(b, e - b);
    body.erase(std::remove(body.begin(), body.end(), '\n'), body.end());
    body.erase(std::remove(body.begin(), body.end(), '\r'), body.end());

    DWORD dlen = 0;
    if (!CryptStringToBinaryA(body.c_str(), (DWORD)body.size(),
                              CRYPT_STRING_BASE64, nullptr, &dlen, nullptr, nullptr))
        return {};
    std::vector<unsigned char> der(dlen);
    if (!CryptStringToBinaryA(body.c_str(), (DWORD)body.size(),
                              CRYPT_STRING_BASE64, der.data(), &dlen, nullptr, nullptr))
        return {};
    der.resize(dlen);
    return der;
}

} // namespace

bool Cert::installTrust()
{
    std::wstring dir = certDir();
    fs::path caPem = fs::path(dir) / L"ca.pem";

    if (!fs::exists(caPem)) {
        CertBundle b = Cert::generate();
        if (b.caPem.empty()) return false;
        std::string narrow(dir.begin(), dir.end());
        if (!Cert::save(b, narrow)) return false;
    }

    auto der = pemToDer(caPem);
    if (der.empty()) return false;

    PCCERT_CONTEXT ctx = CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        der.data(), (DWORD)der.size());
    if (!ctx) return false;

    // CurrentUser\Root — no UAC needed.
    HCERTSTORE store = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W, 0, 0,
        CERT_SYSTEM_STORE_CURRENT_USER, L"ROOT");
    if (!store) { CertFreeCertificateContext(ctx); return false; }

    BOOL ok = CertAddCertificateContextToStore(
        store, ctx, CERT_STORE_ADD_REPLACE_EXISTING, nullptr);

    CertCloseStore(store, 0);
    CertFreeCertificateContext(ctx);
    return ok == TRUE;
}

bool Cert::uninstallTrust()
{
    std::wstring dir = certDir();
    fs::path caPem = fs::path(dir) / L"ca.pem";
    auto der = pemToDer(caPem);
    if (der.empty()) return false;

    PCCERT_CONTEXT ctx = CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        der.data(), (DWORD)der.size());
    if (!ctx) return false;

    HCERTSTORE store = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W, 0, 0,
        CERT_SYSTEM_STORE_CURRENT_USER, L"ROOT");
    if (!store) { CertFreeCertificateContext(ctx); return false; }

    PCCERT_CONTEXT found = CertFindCertificateInStore(
        store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        0, CERT_FIND_EXISTING, ctx, nullptr);
    bool ok = false;
    if (found) {
        ok = CertDeleteCertificateFromStore(found) == TRUE;
    }
    CertCloseStore(store, 0);
    CertFreeCertificateContext(ctx);
    return ok;
}

#else
bool Cert::installTrust() { return false; }
bool Cert::uninstallTrust() { return false; }
#endif

} // namespace nyx
