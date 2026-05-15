#include "core/cert.hpp"

#include <cstdlib>
#include <string>

namespace nyx {

bool Cert::installTrust()
{
    // TODO(phase-1): LoL on Linux runs via Wine. Trust store must be installed
    // INSIDE the Wine prefix so the Riot Client (Win32 process) trusts the CA.
    //
    // Steps:
    //   1. Cert::generate() + save() to ~/.config/Nyx/ca.pem and ca.crt (DER).
    //   2. Detect WINEPREFIX (env or ~/.wine).
    //   3. Add registry key under
    //      HKLM\Software\Microsoft\SystemCertificates\ROOT\Certificates\<thumb>
    //      via 'wine reg add' or by editing system.reg directly.
    //   4. Optional: also push to host trust store
    //      (/usr/local/share/ca-certificates/nyx.crt + update-ca-certificates)
    //      for non-Wine flows.
    return false;
}

bool Cert::uninstallTrust()
{
    return false;
}

} // namespace nyx
