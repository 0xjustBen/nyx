#include "ui/app_controller.hpp"
#include "ui/roster_model.hpp"
#include "ui/presence_model.hpp"
#include "core/proxy.hpp"
#include "core/config_proxy.hpp"
#include "core/launcher.hpp"
#include "core/cert.hpp"
#include "core/config.hpp"
#include "core/roster_store.hpp"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

namespace nyx {

constexpr const char *kLocalhostDomain = "deceive-localhost.molenzwiebel.xyz";
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
};

AppController::AppController(QObject *parent)
    : QObject(parent), d(std::make_unique<Impl>())
{
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

    // Pre-bind chat proxy on a random free port so we can tell ConfigProxy
    // exactly where to point Riot Client. Upstream host is unset until
    // chatServerResolved fires; first incoming connection won't have a
    // valid upstream until then — Riot client retries.
    QString caPath = certDir() + "/ca.pem";
    if (!QFile::exists(caPath)) {
        emit logLine("startup: generating cert artifacts");
        Cert::installTrust();
    }
    if (!d->proxy->start(certDir(), 0, "", 5223)) {
        emit logLine("proxy: pre-bind failed");
    }
    uint16_t chatPort = d->proxy->boundPort() ? d->proxy->boundPort() : kChatProxyPort;
    if (!d->configProxy->start(kLocalhostDomain, chatPort)) {
        d->status = "configproxy error";
    } else {
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

void AppController::quit() { QCoreApplication::quit(); }

} // namespace nyx
