#include "core/cert.hpp"

#import <Security/Security.h>
#import <Foundation/Foundation.h>

namespace nyx {

bool Cert::installTrust()
{
    // TODO(phase-1):
    //   1. Cert::generate() + save() into ~/Library/Application Support/Nyx/ca.pem.
    //   2. Load PEM into SecCertificateRef via SecCertificateCreateWithData.
    //   3. SecTrustSettingsSetTrustSettings(cert, kSecTrustSettingsDomainAdmin, ...).
    //      Requires sudo / authorization prompt. Alternative:
    //      'security add-trusted-cert -d -r trustRoot -k /Library/Keychains/System.keychain <pem>'
    //      via NSTask with admin AuthorizationRef.
    return false;
}

bool Cert::uninstallTrust()
{
    // SecTrustSettingsRemoveTrustSettings + SecItemDelete by SHA-1 of CA.
    return false;
}

} // namespace nyx
