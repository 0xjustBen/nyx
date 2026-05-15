#include "core/cert.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#endif

namespace nyx {

bool Cert::installTrust()
{
#if defined(_WIN32)
    // TODO(phase-1):
    //   1. Cert::generate() + save() to %APPDATA%/Nyx/ca.pem.
    //   2. CertOpenSystemStoreW(0, L"ROOT").
    //   3. CertCreateCertificateContext from DER bytes of CA.
    //   4. CertAddCertificateContextToStore(... CERT_STORE_ADD_REPLACE_EXISTING).
    //   UAC elevation required for HKLM ROOT store; alternative: per-user CurrentUser\Root.
    return false;
#else
    return false;
#endif
}

bool Cert::uninstallTrust()
{
#if defined(_WIN32)
    // CertFindCertificateInStore by SHA1 thumbprint, CertDeleteCertificateFromStore.
    return false;
#else
    return false;
#endif
}

} // namespace nyx
