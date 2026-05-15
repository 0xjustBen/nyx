#pragma once
#include <string>

namespace nyx {

class Patcher {
public:
    // Rewrites Riot Client system.yaml so chat host points at 127.0.0.1.
    // Original saved as system.yaml.nyx-bak alongside.
    static bool patch();
    static bool restore();

    static std::string systemYamlPath();
    static bool patched();
};

} // namespace nyx
