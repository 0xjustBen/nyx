#include "core/launcher.hpp"

#include <QProcess>
#include <QFileInfo>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

namespace nyx {

Launcher::Launcher(QObject *parent) : QObject(parent) {}

QString Launcher::riotClientPath()
{
#if defined(_WIN32)
    // Riot Client install path is registered in HKLM\Software\Riot Games, but
    // the canonical install lives under ProgramData. Cover the common roots.
    QStringList candidates;
    auto add = [&](const QString &p){ if (!p.isEmpty()) candidates << p; };
    add(qEnvironmentVariable("PROGRAMDATA")    + "/Riot Games/Riot Client/RiotClientServices.exe");
    add(qEnvironmentVariable("ProgramFiles")   + "/Riot Games/Riot Client/RiotClientServices.exe");
    add(qEnvironmentVariable("ProgramFiles(x86)") + "/Riot Games/Riot Client/RiotClientServices.exe");
    add(qEnvironmentVariable("LOCALAPPDATA")   + "/Riot Games/Riot Client/RiotClientServices.exe");
    candidates
        << "C:/Riot Games/Riot Client/RiotClientServices.exe"
        << "C:/ProgramData/Riot Games/Riot Client/RiotClientServices.exe"
        << "C:/Program Files/Riot Games/Riot Client/RiotClientServices.exe"
        << "C:/Program Files (x86)/Riot Games/Riot Client/RiotClientServices.exe";
#elif defined(__APPLE__)
    QStringList candidates = {
        "/Applications/Riot Client.app/Contents/MacOS/RiotClientServices",
        QDir::homePath() + "/Applications/Riot Client.app/Contents/MacOS/RiotClientServices",
    };
#else
    QStringList candidates = {
        QDir::homePath() + "/.wine/drive_c/Riot Games/Riot Client/RiotClientServices.exe",
    };
#endif
    for (const QString &p : candidates) if (QFileInfo::exists(p)) return p;
    return {};
}

bool Launcher::launch(uint16_t configProxyPort, const QString &launchProduct, const QString &patchline)
{
    QString path = riotClientPath();
    if (path.isEmpty()) {
        emit log("launcher: RiotClientServices not found");
        return false;
    }

    QStringList args = {
        QString("--client-config-url=http://127.0.0.1:%1").arg(configProxyPort),
    };
    if (!launchProduct.isEmpty())
        args << QString("--launch-product=%1").arg(launchProduct)
             << QString("--launch-patchline=%1").arg(patchline);

    emit log("launcher: " + path + " " + args.join(" "));
    return QProcess::startDetached(path, args);
}

} // namespace nyx
