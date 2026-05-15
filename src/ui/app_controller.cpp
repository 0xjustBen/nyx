#include "ui/app_controller.hpp"
#include "ui/roster_model.hpp"
#include "ui/presence_model.hpp"
#include "core/proxy.hpp"
#include "core/config_proxy.hpp"
#include "core/launcher.hpp"
#include "core/cert.hpp"
#include "core/config.hpp"
#include "core/roster_store.hpp"
#include "core/hosts_file.hpp"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QDateTime>

namespace nyx {

// Use 'localhost' as chat.host. Built-in to Windows resolver (no public
// DNS needed) and our leaf cert already lists DNS:localhost as a SAN.
// Riot Client likely rejects bare IPv4 as a hostname value, so we use the
// DNS literal instead.
constexpr const char *kLocalhostDomain = "localhost";
constexpr uint16_t kChatProxyPort = 5223;

struct AppController::Impl {
    QString mode = "online";
    QString status = "idle";
    bool connected = false;
    RosterModel *roster = nullptr;
    PresenceModel *presence = nullptr;
    std::unique_ptr<ProxyService> proxy;
    std::unique_ptr<ConfigProxy> configProxy;
    std::unique_ptr<Launcher> launcher;
    std::unique_ptr<RosterStore> store;
    Config config;
    QStringList logBuf;  // captures every logLine before UI subscribes
};

AppController::AppController(QObject *parent)
    : QObject(parent), d(std::make_unique<Impl>())
{
    // Capture every emitted logLine so QML can replay them on screen mount.
    connect(this, &AppController::logLine, this,
            [this](const QString &l) {
                d->logBuf.append(QDateTime::currentDateTime().toString("HH:mm:ss.zzz") + " " + l);
                if (d->logBuf.size() > 500) d->logBuf.removeFirst();
            });

    d->roster = new RosterModel(this);
    d->presence = new PresenceModel(this);
    d->proxy = std::make_unique<ProxyService>();
    d->configProxy = std::make_unique<ConfigProxy>();
    d->launcher = std::make_unique<Launcher>();

    // Roster persistence DB next to cert dir.
    QString dbPath = certDir() + "/roster.sqlite";
    d->store = std::make_unique<RosterStore>(dbPath);
    if (d->store->open()) {
        const auto cached = d->store->loadAll();
        for (const auto &f : cached) d->roster->upsert(f);
    }

    auto forwardLog = [this](const QString &l) { emit logLine(l); };
    connect(d->proxy.get(),       &ProxyService::log,  this, forwardLog);
    connect(d->configProxy.get(), &ConfigProxy::log,   this, forwardLog);
    connect(d->launcher.get(),    &Launcher::log,      this, forwardLog);

    connect(d->proxy.get(), &ProxyService::clientConnected,
            this, [this]{ d->connected = true;  emit connectedChanged(); });
    connect(d->proxy.get(), &ProxyService::clientDisconnected,
            this, [this]{ d->connected = false; emit connectedChanged(); });

    // Roster wiring: XMPP S2C parsed events feed RosterModel + persist.
    connect(d->proxy.get(), &ProxyService::rosterItem, this,
            [this](const QString &jid, const QString &name, const QString &tag) {
        Friend f;
        f.jid = jid; f.name = name; f.tag = tag; f.presence = "offline";
        d->roster->upsert(f);
        if (d->store) d->store->upsert(f);
    });
    connect(d->proxy.get(), &ProxyService::presenceUpdate, this,
            [this](const QString &jid, const QString &presence,
                   const QString &game, const QString &activity) {
        d->roster->updatePresence(jid, presence, game, activity);
        if (d->store) {
            // Persist by reading back from model for the joined record.
            Friend f;
            f.jid = jid; f.presence = presence; f.game = game; f.activity = activity;
            d->store->upsert(f);
        }
    });

    // Update upstream when chat host is resolved.
    connect(d->configProxy.get(), &ConfigProxy::chatServerResolved,
            this, [this](const QString &host, uint16_t port) {
        d->proxy->setUpstream(host, port);
    });
}

AppController::~AppController() = default;

QString AppController::certDir()
{
    QString d = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(d);
    return d;
}

void AppController::start()
{
    d->config.load();
    d->proxy->setMode(d->mode);

    QString caPath = certDir() + "/ca.pem";
    if (!QFile::exists(caPath)) {
        emit logLine("startup: cert artifacts absent — generating + installing CA");
        bool ok = Cert::installTrust();
        emit logLine(ok ? "startup: cert installed OK" : "startup: cert install FAILED");
    } else {
        emit logLine("startup: cert artifacts already present at " + caPath);
        // Still try to install in case user uninstalled CA externally.
        Cert::installTrust();
    }

    // No hosts file or DNS dependency — chat.host = 127.0.0.1.

    // Pre-bind chat proxy on random free port so we can tell ConfigProxy
    // exactly where to point Riot Client.
    if (!d->proxy->start(certDir(), 0, "", 5223)) {
        emit logLine("startup: chat proxy pre-bind FAILED (cert files missing or port in use)");
    }
    uint16_t chatPort = d->proxy->boundPort();
    if (chatPort == 0) {
        emit logLine("startup: chat proxy bound port = 0 — fallback to 5223 (likely broken)");
        chatPort = kChatProxyPort;
    } else {
        emit logLine(QString("startup: chat proxy bound on 127.0.0.1:%1").arg(chatPort));
    }

    if (!d->configProxy->start(kLocalhostDomain, chatPort)) {
        emit logLine("startup: configproxy start FAILED");
        d->status = "configproxy error";
    } else {
        emit logLine(QString("startup: configproxy http://127.0.0.1:%1 — chat port :%2 — localhost-domain %3")
                         .arg(d->configProxy->port()).arg(chatPort).arg(kLocalhostDomain));
        d->status = QString("configproxy :%1 · chat :%2")
                        .arg(d->configProxy->port()).arg(chatPort);
    }
    emit statusChanged();
}

QString AppController::mode() const     { return d->mode; }
QString AppController::status() const   { return d->status; }
bool    AppController::connected() const { return d->connected; }
RosterModel *AppController::roster() const { return d->roster; }

void AppController::setMode(const QString &m)
{
    if (m == d->mode) return;
    d->mode = m;
    d->proxy->setMode(m);
    // Push a fresh presence stanza so friends see the new state immediately,
    // without waiting for the game client to re-broadcast.
    d->proxy->resendPresence();
    emit modeChanged();
}

bool AppController::installCert()
{
    bool ok = Cert::installTrust();
    emit logLine(ok ? "cert: installed into user trust store"
                    : "cert: install FAILED — try Run as Administrator");
    return ok;
}

bool AppController::uninstallCert()
{
    bool ok = Cert::uninstallTrust();
    emit logLine(ok ? "cert: removed from user trust store"
                    : "cert: uninstall failed (not present?)");
    return ok;
}

void AppController::launchRiot()
{
    launchProduct("");  // Riot Client home / launcher
}

void AppController::launchProduct(const QString &product)
{
    if (!d->configProxy->port()) {
        emit logLine("launch: configproxy not running");
        return;
    }
    QString caPath = certDir() + "/ca.pem";
    if (!QFile::exists(caPath)) {
        emit logLine("launch: cert not generated yet — generating & installing");
        Cert::installTrust();
    }
    QString launchProd = product;
    QString patchline = "live";
    if (product == "league" || product == "lol")        launchProd = "league_of_legends";
    else if (product == "valorant" || product == "val") launchProd = "valorant";
    else if (product == "lor")                          launchProd = "bacon";
    else if (product == "2xko")                         launchProd = "lion";
    d->launcher->launch(d->configProxy->port(), launchProd, patchline);
}

void AppController::killRiotClients()
{
    Launcher::killExistingRiotClients();
    emit logLine("riot client processes killed");
}

void AppController::pause(bool on)
{
    if (on) {
        if (d->proxy)       d->proxy->stop();
        if (d->configProxy) d->configProxy->stop();
        d->status = "paused";
        emit logLine("nyx: paused");
    } else {
        start();
        emit logLine("nyx: resumed");
    }
    emit statusChanged();
}

bool AppController::paused() const
{
    return d->status == "paused";
}

void AppController::setRegion(const QString &r)
{
    d->config.region = r.toStdString();
    d->config.save();
    emit logLine("region set: " + r);
}

QString AppController::region() const
{
    return QString::fromStdString(d->config.region);
}

void AppController::setAutostart(bool on)
{
    d->config.startMinimized = on; // reusing field
    d->config.save();

    // Platform autostart wiring.
#if defined(_WIN32)
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    QString exePath = QCoreApplication::applicationFilePath().replace('/', '\\');
    if (on) reg.setValue("Nyx", "\"" + exePath + "\"");
    else    reg.remove("Nyx");
#elif defined(__APPLE__)
    // macOS: write a LaunchAgent plist.
    QString plistDir = QDir::homePath() + "/Library/LaunchAgents";
    QDir().mkpath(plistDir);
    QString plistPath = plistDir + "/io.nyx.app.plist";
    if (on) {
        QFile f(plistPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QString exe = QCoreApplication::applicationFilePath();
            f.write(QString(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
                "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
                "<plist version=\"1.0\">"
                "<dict>"
                "<key>Label</key><string>io.nyx.app</string>"
                "<key>ProgramArguments</key><array><string>%1</string></array>"
                "<key>RunAtLoad</key><true/>"
                "</dict></plist>").arg(exe).toUtf8());
        }
    } else {
        QFile::remove(plistPath);
    }
#else
    QString autostartDir = QDir::homePath() + "/.config/autostart";
    QDir().mkpath(autostartDir);
    QString desktopPath = autostartDir + "/nyx.desktop";
    if (on) {
        QFile f(desktopPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QString exe = QCoreApplication::applicationFilePath();
            f.write(QString(
                "[Desktop Entry]\n"
                "Type=Application\n"
                "Name=Nyx\n"
                "Exec=%1\n"
                "X-GNOME-Autostart-enabled=true\n").arg(exe).toUtf8());
        }
    } else {
        QFile::remove(desktopPath);
    }
#endif
}

bool AppController::autostart() const
{
#if defined(_WIN32)
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    return reg.contains("Nyx");
#elif defined(__APPLE__)
    return QFile::exists(QDir::homePath() + "/Library/LaunchAgents/io.nyx.app.plist");
#else
    return QFile::exists(QDir::homePath() + "/.config/autostart/nyx.desktop");
#endif
}

void AppController::notifyMode()
{
    emit logLine("mode notify: " + d->mode);
}

QStringList AppController::initialLog() const
{
    return d->logBuf;
}

void AppController::quit() { QCoreApplication::quit(); }

} // namespace nyx
