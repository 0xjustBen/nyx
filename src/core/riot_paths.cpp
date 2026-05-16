#include "core/riot_paths.hpp"

#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace nyx {

std::string RiotPaths::riotClientRoot()
{
#if defined(_WIN32)
    return "C:/Riot Games/Riot Client";
#elif defined(__APPLE__)
    return "/Applications/League of Legends.app/Contents/LoL/RiotClient";
#else
    const char *h = std::getenv("HOME");
    if (!h) return {};
    return std::string(h) + "/.wine/drive_c/Riot Games/Riot Client";
#endif
}

std::string RiotPaths::leagueRoot()
{
#if defined(_WIN32)
    return "C:/Riot Games/League of Legends";
#elif defined(__APPLE__)
    return "/Applications/League of Legends.app/Contents/LoL";
#else
    const char *h = std::getenv("HOME");
    if (!h) return {};
    return std::string(h) + "/.wine/drive_c/Riot Games/League of Legends";
#endif
}

std::string RiotPaths::systemYaml()
{
    fs::path p = leagueRoot();
    p /= "system.yaml";
    if (fs::exists(p)) return p.string();
    return {};
}

std::vector<std::string> RiotPaths::chatHosts()
{
    return {
        // RFC 6761 reserved — every compliant resolver returns loopback for
        // *.localhost without consulting DNS.
        "nyx.localhost",
        "*.localhost",
        // Public DNS fallback (sslip.io wildcard, A → 127.0.0.1).
        "127.0.0.1.sslip.io",
        // Legacy Deceive domain — keep so old certs still validate.
        "deceive-localhost.molenzwiebel.xyz",
        "chat.na1.lol.riotgames.com",
        "chat.euw1.lol.riotgames.com",
        "chat.eun1.lol.riotgames.com",
        "chat.kr.lol.riotgames.com",
        "chat.br1.lol.riotgames.com",
        "chat.la1.lol.riotgames.com",
        "chat.la2.lol.riotgames.com",
        "chat.oc1.lol.riotgames.com",
        "chat.ru.lol.riotgames.com",
        "chat.tr1.lol.riotgames.com",
        "chat.jp1.lol.riotgames.com",
        "chat.ph2.lol.riotgames.com",
        "chat.sg2.lol.riotgames.com",
        "chat.th2.lol.riotgames.com",
        "chat.tw2.lol.riotgames.com",
        "chat.vn2.lol.riotgames.com",
    };
}

} // namespace nyx
