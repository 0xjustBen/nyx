#include "ui/app_controller.hpp"
#include "ui/roster_model.hpp"
#include "ui/presence_model.hpp"
#include "core/proxy.hpp"
#include "core/config_proxy.hpp"
#include "core/launcher.hpp"
#include "core/cert.hpp"
#include "core/config.hpp"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>

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

    auto forwardLog = [this](const QString &l) { emit logLine(l); };
    connect(d->proxy.get(),       &ProxyService::log,  this, forwardLog);
    connect(d->configProxy.get(), &ConfigProxy::log,   this, forwardLog);
    connect(d->launcher.get(),    &Launcher::log,      this, forwardLog);

    connect(d->proxy.get(), &ProxyService::clientConnected,
            this, [this]{ d->connected = true;  emit connectedChanged(); });
    connect(d->proxy.get(), &ProxyService::clientDisconnected,
            this, [this]{ d->connected = false; emit connectedChanged(); });

    // When ConfigProxy learns the real chat host from clientconfig response,
    // start the TLS chat proxy targeting it.
    connect(d->configProxy.get(), &ConfigProxy::chatServerResolved,
            this, [this](const QString &host, uint16_t port) {
        if (!d->proxy->start(certDir(), kChatProxyPort, host, port)) {
            emit logLine("proxy: start failed");
        }
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
    if (!d->configProxy->start(kLocalhostDomain, kChatProxyPort)) {
        d->status = "configproxy error";
    } else {
        d->status = QString("configproxy on :%1").arg(d->configProxy->port());
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
    emit modeChanged();
}

void AppController::installCert()   { Cert::installTrust(); }
void AppController::uninstallCert() { Cert::uninstallTrust(); }

void AppController::launchRiot()
{
    if (!d->configProxy->port()) {
        emit logLine("launch: configproxy not running");
        return;
    }
    d->launcher->launch(d->configProxy->port());
}

void AppController::quit() { QCoreApplication::quit(); }

} // namespace nyx
