#pragma once
#include <string>

namespace nyx {

struct Config {
    std::string mode = "online";
    std::string region = "na1";
    bool startMinimized = false;
    bool autoPatch = true;

    static std::string path();

    bool load();
    bool save() const;
};

} // namespace nyx
