#pragma once
#include <string>
#include <vector>

namespace nyx {

struct CertBundle {
    std::vector<unsigned char> caPem;
    std::vector<unsigned char> caKeyPem;
    std::vector<unsigned char> leafPem;
    std::vector<unsigned char> leafKeyPem;
};

class Cert {
public:
    // Generate root CA + leaf cert for chat.<region>.lol.riotgames.com SANs.
    static CertBundle generate();

    static bool save(const CertBundle &b, const std::string &dir);
    static bool load(CertBundle &out, const std::string &dir);

    // Install CA into OS trust store. Implemented per-platform.
    static bool installTrust();
    static bool uninstallTrust();
};

} // namespace nyx
