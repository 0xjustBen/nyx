#include "core/patcher.hpp"
#include "core/riot_paths.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>

namespace fs = std::filesystem;

namespace nyx {

static fs::path backupPath(const fs::path &p) { return p.string() + ".nyx-bak"; }

std::string Patcher::systemYamlPath()
{
    return RiotPaths::systemYaml();
}

bool Patcher::patched()
{
    fs::path p = systemYamlPath();
    return fs::exists(backupPath(p));
}

bool Patcher::patch()
{
    fs::path p = systemYamlPath();
    if (p.empty() || !fs::exists(p)) return false;
    if (!fs::exists(backupPath(p))) {
        std::error_code ec;
        fs::copy_file(p, backupPath(p), ec);
        if (ec) return false;
    }
    std::ifstream in(p);
    std::stringstream ss; ss << in.rdbuf();
    std::string yaml = ss.str();

    // Rewrite any chat_host: chat.*.lol.riotgames.com to 127.0.0.1.
    static const std::regex re(R"((chat_host:\s*)([^\s#]+))");
    yaml = std::regex_replace(yaml, re, "$1\"127.0.0.1\"");

    std::ofstream out(p, std::ios::trunc);
    out << yaml;
    return out.good();
}

bool Patcher::restore()
{
    fs::path p = systemYamlPath();
    fs::path b = backupPath(p);
    if (!fs::exists(b)) return false;
    std::error_code ec;
    fs::copy_file(b, p, fs::copy_options::overwrite_existing, ec);
    if (ec) return false;
    fs::remove(b, ec);
    return true;
}

} // namespace nyx
