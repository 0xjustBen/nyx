#include "core/hosts_file.hpp"

#include <QFile>
#include <QStringList>
#include <QTextStream>

namespace nyx {

static const char *kBegin = "# BEGIN NYX";
static const char *kEnd   = "# END NYX";

QString HostsFile::path()
{
#if defined(_WIN32)
    return "C:/Windows/System32/drivers/etc/hosts";
#else
    return "/etc/hosts";
#endif
}

static QString readAll(const QString &p)
{
    QFile f(p);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QString::fromUtf8(f.readAll());
}

static bool writeAll(const QString &p, const QString &content)
{
    QFile f(p);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    return f.write(content.toUtf8()) == content.toUtf8().size();
}

bool HostsFile::ensure(const QString &domain)
{
    QString cur = readAll(path());
    if (cur.contains(domain)) return true;  // already mapped (idempotent)

    // Strip any prior NYX block.
    int b = cur.indexOf(kBegin);
    int e = cur.indexOf(kEnd);
    if (b >= 0 && e > b) {
        cur.remove(b, e + (int)strlen(kEnd) - b);
    }

    QString block = QString("\n%1\n127.0.0.1 %2\n%3\n").arg(kBegin, domain, kEnd);
    cur += block;
    return writeAll(path(), cur);
}

bool HostsFile::remove()
{
    QString cur = readAll(path());
    int b = cur.indexOf(kBegin);
    int e = cur.indexOf(kEnd);
    if (b < 0 || e <= b) return true;
    cur.remove(b, e + (int)strlen(kEnd) - b);
    return writeAll(path(), cur);
}

bool HostsFile::contains(const QString &domain)
{
    return readAll(path()).contains(domain);
}

} // namespace nyx
