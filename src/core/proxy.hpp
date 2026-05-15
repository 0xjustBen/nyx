#pragma once
#include <QObject>
#include <QString>
#include <cstdint>
#include <memory>

namespace nyx {

class ProxyService : public QObject {
    Q_OBJECT
public:
    explicit ProxyService(QObject *parent = nullptr);
    ~ProxyService() override;

    bool start(const QString &certDir, uint16_t listenPort = 5223,
               const QString &upstreamHost = "chat.na1.lol.riotgames.com",
               uint16_t upstreamPort = 5223);
    void stop();

    void setMode(const QString &mode);
    QString mode() const;

signals:
    void log(const QString &line);
    void clientConnected();
    void clientDisconnected();
    void bytesPumped(qint64 c2s, qint64 s2c);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace nyx
