#pragma once
#include <QString>

namespace nyx {

// Manage the system hosts file mapping for our localhost-domain. Writes a
// single line `127.0.0.1 <domain>` between BEGIN/END NYX markers so we can
// cleanly remove it on uninstall. Requires admin/root.
class HostsFile {
public:
    static QString path();          // platform-specific hosts path
    static bool ensure(const QString &domain);
    static bool remove();
    static bool contains(const QString &domain);
};

} // namespace nyx
