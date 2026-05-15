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

    bool start(const QString &certDir, uint16_t listenPort = 0,
               const QString &upstreamHost = "chat.na1.lol.riotgames.com",
               uint16_t upstreamPort = 5223);
    void stop();

    // Port we actually bound (resolved when listenPort=0 picks a free one).
    uint16_t boundPort() const;

    // Update upstream after start() — used when chat host is learned async.
    void setUpstream(const QString &host, uint16_t port);

    void setMode(const QString &mode);
    QString mode() const;

    // Synthesize a <presence> stanza with the current mode and send it to the
    // upstream chat server. Used after Mode changes mid-session so friends see
    // the update immediately without the user toggling in-game state.
    void resendPresence();

signals:
    void log(const QString &line);
    void clientConnected();
    void clientDisconnected();
    void bytesPumped(qint64 c2s, qint64 s2c);
    void rosterItem(const QString &jid, const QString &name, const QString &tag);
    void presenceUpdate(const QString &jid, const QString &presence,
                        const QString &game, const QString &activity);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace nyx
