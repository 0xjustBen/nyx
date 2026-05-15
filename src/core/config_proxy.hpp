#pragma once
#include <QObject>
#include <QString>
#include <cstdint>
#include <memory>

namespace nyx {

// Reverse-proxy for clientconfig.rpg.riotgames.com.
// Rewrites the chat host/port/affinities in the JSON response so the Riot
// Client connects to our local chat proxy. Riot Client is launched with
// --client-config-url=http://127.0.0.1:<port> pointing here.
class ConfigProxy : public QObject {
    Q_OBJECT
public:
    explicit ConfigProxy(QObject *parent = nullptr);
    ~ConfigProxy() override;

    bool start(const QString &localhostDomain, uint16_t chatProxyPort);
    void stop();

    // Update the chat port reported to Riot Client (called once we know what
    // port QSslServer actually bound).
    void setChatPort(uint16_t port);

    uint16_t port() const;

signals:
    // Emitted once Riot's clientconfig response is parsed and we learn the
    // real chat host/port for this player's affinity.
    void chatServerResolved(const QString &host, uint16_t port);
    void log(const QString &line);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace nyx
