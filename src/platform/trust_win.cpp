#include "core/cert.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <objbase.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;
#endif

namespace nyx {

#if defined(_WIN32)
namespace {

// Roaming AppData root for Nyx cert artifacts. Matches QStandardPaths::AppDataLocation
// on Windows (FOLDERID_RoamingAppData / OrgName / AppName).
fs::path certDirPath()
{
    PWSTR roaming = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roaming);
    fs::path dir;
    if (SUCCEEDED(hr) && roaming) {
        dir = roaming;
    } else {
        dir = fs::temp_directory_path();
    }
    if (roaming) CoTaskMemFree(roaming);
    dir /= L"nyx";
    dir /= L"Nyx";
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir;
}

std::vector<unsigned char> readBinary(const fs::path &p)
{
    std::ifstream in(p, std::ios::binary);
    if (!in) return {};
    std::stringstream ss; ss << in.rdbuf();
    std::string s = ss.str();
    return std::vector<unsigned char>(s.begin(), s.end());
}

std::vector<unsigned char> pemToDer(const fs::path &pem)
{
    auto bytes = readBinary(pem);
    if (bytes.empty()) return {};
    std::string s(bytes.begin(), bytes.end());
    auto b = s.find("-----BEGIN CERTIFICATE-----");
    auto e = s.find("-----END CERTIFICATE-----");
    if (b == std::string::npos || e == std::string::npos) return {};
    b += sizeof("-----BEGIN CERTIFICATE-----") - 1;
    std::string body = s.substr(b, e - b);
    body.erase(std::remove(body.begin(), body.end(), '\n'), body.end());
    body.erase(std::remove(body.begin(), body.end(), '\r'), body.end());
    body.erase(std::remove(body.begin(), body.end(), ' '),  body.end());

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

PCCERT_CONTEXT toCertContext(const std::vector<unsigned char> &der)
{
    return CertCreateCertificateContext(
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        der.data(), (DWORD)der.size());
}

} // namespace

bool Cert::installTrust()
{
    fs::path dir   = certDirPath();
    fs::path caPem = dir / L"ca.pem";

    if (!fs::exists(caPem)) {
        CertBundle b = Cert::generate();
        if (b.caPem.empty()) return false;
        if (!Cert::save(b, dir.u8string())) return false;
    }

    auto der = pemToDer(caPem);
    if (der.empty()) return false;

    PCCERT_CONTEXT ctx = toCertContext(der);
    if (!ctx) return false;

    // CurrentUser\Root — no UAC needed. SChannel + .NET + Chromium all consult it.
    HCERTSTORE store = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W,
        0, 0,
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
    fs::path dir   = certDirPath();
    fs::path caPem = dir / L"ca.pem";
    auto der = pemToDer(caPem);
    if (der.empty()) return false;

    PCCERT_CONTEXT ctx = toCertContext(der);
    if (!ctx) return false;

    HCERTSTORE store = CertOpenStore(
        CERT_STORE_PROV_SYSTEM_W, 0, 0,
        CERT_SYSTEM_STORE_CURRENT_USER, L"ROOT");
    if (!store) { CertFreeCertificateContext(ctx); return false; }

    bool ok = false;
    PCCERT_CONTEXT found = CertFindCertificateInStore(
        store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        0, CERT_FIND_EXISTING, ctx, nullptr);
    if (found) {
        ok = CertDeleteCertificateFromStore(found) == TRUE;
    }
    CertCloseStore(store, 0);
    CertFreeCertificateContext(ctx);
    return ok;
}

#else
bool Cert::installTrust()   { return false; }
bool Cert::uninstallTrust() { return false; }
#endif

} // namespace nyx
