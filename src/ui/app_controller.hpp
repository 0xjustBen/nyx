#pragma once
#include <QObject>
#include <QString>
#include <memory>

#include "ui/roster_model.hpp"

namespace nyx {

class ProxyService;
class PresenceModel;

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(RosterModel* roster READ roster CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    void start();
    static QString certDir();

    QString mode() const;
    void setMode(const QString &mode);
    QString status() const;
    bool connected() const;
    RosterModel *roster() const;

    Q_INVOKABLE void installCert();
    Q_INVOKABLE void uninstallCert();
    Q_INVOKABLE void patchClient();
    Q_INVOKABLE void restoreClient();
    Q_INVOKABLE void quit();

signals:
    void modeChanged();
    void statusChanged();
    void connectedChanged();
    void logLine(const QString &line);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace nyx
