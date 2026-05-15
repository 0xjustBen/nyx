#pragma once
#include <string>
#include <vector>

namespace nyx {

class RiotPaths {
public:
    static std::string riotClientRoot();
    static std::string leagueRoot();
    static std::string systemYaml();

    // Candidate hosts to point at 127.0.0.1.
    static std::vector<std::string> chatHosts();
};

} // namespace nyx
