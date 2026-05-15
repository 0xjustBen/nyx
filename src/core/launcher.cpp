#include "core/launcher.hpp"

#include <QProcess>
#include <QFileInfo>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QThread>

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

static void killProc(const QString &name)
{
#if defined(_WIN32)
    QProcess::execute("taskkill", {"/F", "/T", "/IM", name});
#else
    QProcess::execute("pkill", {"-f", name});
#endif
}

bool Launcher::killExistingRiotClients()
{
    static const QStringList names = {
        "RiotClientServices.exe",
        "RiotClientUx.exe",
        "RiotClientUxRender.exe",
        "RiotClientCrashHandler.exe",
        "LeagueClient.exe",
        "LeagueClientUx.exe",
        "LeagueClientUxRender.exe",
        "VALORANT.exe",
        "VALORANT-Win64-Shipping.exe",
        "RiotGames.exe",
    };
    for (const QString &n : names) killProc(n);
    // Give the OS a moment to release the lock files Riot Client holds.
    QThread::msleep(800);
    return true;
}

bool Launcher::launch(uint16_t configProxyPort, const QString &launchProduct, const QString &patchline)
{
    QString path = riotClientPath();
    if (path.isEmpty()) {
        emit log("launcher: RiotClientServices not found");
        return false;
    }

    // CRITICAL: Riot Client is a singleton. If one is already running, our
    // --client-config-url arg is silently ignored and the existing instance
    // keeps talking to the real chat server. Kill anything Riot first.
    emit log("launcher: killing existing Riot Client processes");
    killExistingRiotClients();

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
