#include "ui/app_controller.hpp"
#include "ui/roster_model.hpp"
#include "ui/presence_model.hpp"
#include "core/proxy.hpp"
#include "core/cert.hpp"
#include "core/patcher.hpp"
#include "core/config.hpp"

#include <QCoreApplication>

namespace nyx {

struct AppController::Impl {
    QString mode = "online";
    QString status = "idle";
    bool connected = false;
    RosterModel *roster = nullptr;
    PresenceModel *presence = nullptr;
    std::unique_ptr<ProxyService> proxy;
    Config config;
};

AppController::AppController(QObject *parent)
    : QObject(parent), d(std::make_unique<Impl>())
{
    d->roster = new RosterModel(this);
    d->presence = new PresenceModel(this);
    d->proxy = std::make_unique<ProxyService>();
}

AppController::~AppController() = default;

void AppController::start()
{
    d->config.load();
    d->proxy->setMode(d->mode.toStdString());
    d->proxy->onStanza([this](const std::string &kind) {
        emit logLine(QString::fromStdString(kind));
    });
    d->proxy->onConnected([this](bool up) {
        d->connected = up;
        emit connectedChanged();
    });
    d->proxy->start(5223);
    d->status = "running";
    emit statusChanged();
}

QString AppController::mode() const { return d->mode; }
QString AppController::status() const { return d->status; }
bool AppController::connected() const { return d->connected; }
RosterModel *AppController::roster() const { return d->roster; }

void AppController::setMode(const QString &m)
{
    if (m == d->mode) return;
    d->mode = m;
    d->proxy->setMode(m.toStdString());
    emit modeChanged();
}

void AppController::installCert()   { Cert::installTrust(); }
void AppController::uninstallCert() { Cert::uninstallTrust(); }
void AppController::patchClient()   { Patcher::patch(); }
void AppController::restoreClient() { Patcher::restore(); }
void AppController::quit()          { QCoreApplication::quit(); }

} // namespace nyx
