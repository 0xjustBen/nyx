#include "core/cert.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace nyx {
namespace {

fs::path xdgConfigDir()
{
    if (const char *x = std::getenv("XDG_CONFIG_HOME")) return fs::path(x) / "Nyx";
    if (const char *h = std::getenv("HOME"))            return fs::path(h) / ".config" / "Nyx";
    return fs::temp_directory_path() / "Nyx";
}

fs::path winePrefix()
{
    if (const char *w = std::getenv("WINEPREFIX")) return w;
    if (const char *h = std::getenv("HOME"))       return fs::path(h) / ".wine";
    return {};
}

bool runCmd(const std::string &cmd)
{
    return std::system(cmd.c_str()) == 0;
}

} // namespace

bool Cert::installTrust()
{
    fs::path dir = xdgConfigDir();
    fs::path caPem = dir / "ca.pem";

    if (!fs::exists(caPem)) {
        CertBundle b = Cert::generate();
        if (b.caPem.empty()) return false;
        if (!Cert::save(b, dir.string())) return false;
    }

    // Convert PEM → DER for Wine registry import via `wine certutil`.
    fs::path caDer = dir / "ca.crt";
    runCmd("openssl x509 -in '" + caPem.string() +
           "' -outform DER -out '" + caDer.string() + "'");

    fs::path prefix = winePrefix();
    if (prefix.empty() || !fs::exists(prefix)) return false;

    // Add to the Wine prefix's Root store using certutil (shipped with Wine).
    std::string env = "WINEPREFIX='" + prefix.string() + "' ";
    bool ok = runCmd(env + "wine certutil.exe -addstore -user Root '" + caDer.string() + "' >/dev/null 2>&1");

    // Best-effort: also add to host trust for non-Wine flows.
    fs::path hostCrt = "/usr/local/share/ca-certificates/nyx.crt";
    std::error_code ec;
    fs::copy_file(caDer, hostCrt, fs::copy_options::overwrite_existing, ec);
    if (!ec) runCmd("sudo -n update-ca-certificates >/dev/null 2>&1");

    return ok;
}

bool Cert::uninstallTrust()
{
    fs::path prefix = winePrefix();
    if (prefix.empty()) return false;
    std::string env = "WINEPREFIX='" + prefix.string() + "' ";
    runCmd(env + "wine certutil.exe -delstore -user Root 'Nyx Local Root CA' >/dev/null 2>&1");

    std::error_code ec;
    fs::remove("/usr/local/share/ca-certificates/nyx.crt", ec);
    runCmd("sudo -n update-ca-certificates >/dev/null 2>&1");
    return true;
}

} // namespace nyx
