#include "core/proxy.hpp"
#include "core/xmpp.hpp"

#include <QSslServer>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslKey>
#include <QFile>
#include <QPointer>
#include <QDir>
#include <QHash>

#include <atomic>

namespace nyx {

namespace {

QByteArray readAll(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    return f.readAll();
}

} // namespace

struct ProxyService::Impl {
    QSslServer *server = nullptr;
    QString upstreamHost;
    uint16_t upstreamPort = 5223;
    QSslConfiguration serverCfg;
    QString mode = "online";
    QString userJid;
    XmppRewriter rewriter;
    std::atomic<qint64> c2sBytes{0};
    std::atomic<qint64> s2cBytes{0};

    struct Conn {
        QSslSocket *client = nullptr;
        QSslSocket *upstream = nullptr;
        bool upstreamReady = false;
        QByteArray pending; // c2s buffered before upstream encrypted
    };
    QHash<QSslSocket *, Conn *> byClient;
    QHash<QSslSocket *, Conn *> byUpstream;
};

ProxyService::ProxyService(QObject *parent)
    : QObject(parent), d(std::make_unique<Impl>()) {}

ProxyService::~ProxyService() { stop(); }

bool ProxyService::start(const QString &certDir, uint16_t listenPort,
                         const QString &upstreamHost, uint16_t upstreamPort)
{
    if (d->server) return false;

    QDir dir(certDir);
    QByteArray leafPem = readAll(dir.filePath("leaf.pem"));
    QByteArray leafKey = readAll(dir.filePath("leaf.key"));
    QByteArray caPem   = readAll(dir.filePath("ca.pem"));
    if (leafPem.isEmpty() || leafKey.isEmpty()) {
        emit log("cert: leaf.pem/leaf.key missing in " + certDir);
        return false;
    }

    QSslCertificate cert(leafPem, QSsl::Pem);
    QSslKey key(leafKey, QSsl::Ec, QSsl::Pem);
    if (cert.isNull() || key.isNull()) {
        emit log("cert: parse failed");
        return false;
    }

    QSslConfiguration cfg = QSslConfiguration::defaultConfiguration();
    cfg.setLocalCertificate(cert);
    cfg.setPrivateKey(key);
    if (!caPem.isEmpty()) {
        cfg.setCaCertificates(QSslCertificate::fromData(caPem, QSsl::Pem));
    }
    cfg.setPeerVerifyMode(QSslSocket::VerifyNone);
    d->serverCfg = cfg;
    d->upstreamHost = upstreamHost;
    d->upstreamPort = upstreamPort;

    d->server = new QSslServer(this);
    d->server->setSslConfiguration(cfg);

    connect(d->server, &QSslServer::pendingConnectionAvailable, this, [this] {
        while (auto *sock = qobject_cast<QSslSocket *>(d->server->nextPendingConnection())) {
            auto *c = new Impl::Conn{sock, nullptr, false, {}};
            d->byClient.insert(sock, c);
            emit log(QString("client: incoming %1").arg(sock->peerAddress().toString()));
            emit clientConnected();

            auto teardown = [this, c] {
                if (!c) return;
                // Idempotent — second call from upstream-disconnected is a no-op.
                if (c->client) {
                    d->byClient.remove(c->client);
                    c->client->disconnect();
                    c->client->deleteLater();
                    c->client = nullptr;
                }
                if (c->upstream) {
                    d->byUpstream.remove(c->upstream);
                    c->upstream->disconnect();
                    c->upstream->deleteLater();
                    c->upstream = nullptr;
                }
                delete c;
            };

            connect(sock, &QSslSocket::encrypted, this, [this, c] {
                if (!c->client) return;
                d->rewriter.reset(); // fresh stream buffer per connection
                emit log("client: TLS up — dialing upstream " + d->upstreamHost);
                c->upstream = new QSslSocket(this);
                d->byUpstream.insert(c->upstream, c);
                c->upstream->setProtocol(QSsl::TlsV1_2OrLater);
                // Pin upstream to system trust store so an attacker on the
                // local network can't MITM the real Riot chat server. We
                // still set VerifyPeer; sslErrors handler logs but does NOT
                // ignore unknown failures.
                c->upstream->setPeerVerifyMode(QSslSocket::VerifyPeer);
                c->upstream->setPeerVerifyName(d->upstreamHost);
                connect(c->upstream, &QSslSocket::encrypted, this, [this, c] {
                    if (!c->upstream) return;
                    c->upstreamReady = true;
                    emit log("upstream: TLS up");
                    if (!c->pending.isEmpty()) {
                        c->upstream->write(c->pending);
                        d->c2sBytes += c->pending.size();
                        c->pending.clear();
                    }
                });
                connect(c->upstream, &QSslSocket::readyRead, this, [this, c] {
                    if (!c->upstream) return;
                    QByteArray b = c->upstream->readAll();
                    d->s2cBytes += b.size();
                    if (c->client && c->client->state() == QAbstractSocket::ConnectedState)
                        c->client->write(b);
                    // Capture user's bound JID from any inbound `to='<full-jid>'`
                    // attribute so we can synthesize valid presence later.
                    if (d->userJid.isEmpty()) {
                        int t = b.indexOf("to='");
                        if (t < 0) t = b.indexOf("to=\"");
                        if (t >= 0) {
                            int s = t + 4;
                            char q = (b[t + 3] == '\'') ? '\'' : '"';
                            int e = b.indexOf(q, s);
                            if (e > s) {
                                d->userJid = QString::fromUtf8(b.mid(s, e - s));
                                emit log("bound jid: " + d->userJid);
                            }
                        }
                    }
                    // Observe roster + presence for UI without altering forwarded bytes.
                    d->rewriter.observeS2C(b);
                    for (auto &ev : d->rewriter.drainEvents()) {
                        if (ev.kind == XmppRewriter::Event::RosterItem)
                            emit rosterItem(ev.f.jid, ev.f.name, ev.f.tag);
                        else if (ev.kind == XmppRewriter::Event::PresenceUpdate)
                            emit presenceUpdate(ev.f.jid, ev.f.presence, ev.f.game, ev.f.activity);
                    }
                    emit bytesPumped(d->c2sBytes, d->s2cBytes);
                });
                connect(c->upstream, &QSslSocket::disconnected, this, [this, c] {
                    emit log("upstream: disconnected");
                    if (c->client && c->client->state() == QAbstractSocket::ConnectedState)
                        c->client->disconnectFromHost();
                });
                connect(c->upstream, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
                        this, [this, c](const QList<QSslError> &errs) {
                    if (!c->upstream) return;
                    for (const auto &e : errs) emit log("upstream ssl: " + e.errorString());
                    // Do NOT auto-ignore — VerifyPeer means real errors abort the
                    // upstream connection, which is exactly what we want.
                });
                c->upstream->connectToHostEncrypted(d->upstreamHost, d->upstreamPort);
            });
            connect(sock, &QSslSocket::readyRead, this, [this, c] {
                if (!c->client) return;
                QByteArray raw = c->client->readAll();
                QByteArray b = d->rewriter.rewriteC2S(raw);
                if (c->upstreamReady && c->upstream) {
                    c->upstream->write(b);
                    d->c2sBytes += b.size();
                    emit bytesPumped(d->c2sBytes, d->s2cBytes);
                } else {
                    c->pending.append(b);
                }
            });
            connect(sock, &QSslSocket::disconnected, this, [this, c, teardown] {
                emit log("client: disconnected");
                emit clientDisconnected();
                teardown();
            });
            connect(sock, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
                    this, [this, c](const QList<QSslError> &errs) {
                for (const auto &e : errs) emit log("client ssl: " + e.errorString());
                if (c->client) c->client->ignoreSslErrors();
            });
        }
    });

    // Try IPv4 localhost first; fall back to dual-stack Any (Riot may resolve
    // the localhost-domain to ::1 on some Windows configs).
    if (!d->server->listen(QHostAddress::LocalHost, listenPort) &&
        !d->server->listen(QHostAddress::Any, listenPort)) {
        emit log(QString("listen failed on %1: %2").arg(listenPort).arg(d->server->errorString()));
        d->server->deleteLater();
        d->server = nullptr;
        return false;
    }
    emit log(QString("listening on 127.0.0.1:%1, upstream %2:%3")
                 .arg(d->server->serverPort()).arg(d->upstreamHost).arg(d->upstreamPort));
    return true;
}

uint16_t ProxyService::boundPort() const
{
    return d->server ? (uint16_t)d->server->serverPort() : 0;
}

void ProxyService::setUpstream(const QString &host, uint16_t port)
{
    d->upstreamHost = host;
    d->upstreamPort = port;
    emit log(QString("upstream set: %1:%2").arg(host).arg(port));
}

void ProxyService::stop()
{
    if (!d->server) return;
    d->server->close();
    d->server->deleteLater();
    d->server = nullptr;
}

void ProxyService::setMode(const QString &m) { d->mode = m; d->rewriter.setMode(modeFromString(m)); }
QString ProxyService::mode() const { return d->mode; }

void ProxyService::resendPresence()
{
    if (d->byClient.isEmpty()) {
        emit log("presence: no active client connection");
        return;
    }
    Mode m = modeFromString(d->mode);

    QByteArray fromAttr = d->userJid.isEmpty() ? QByteArray()
                         : (" from='" + d->userJid.toUtf8() + "'");

    QByteArray stanza;
    if (m == Mode::Offline) {
        // Invisible: send <presence type='unavailable'> to mark us offline
        // server-side. Server broadcasts unavailable to roster. Friends see
        // offline. Local Riot Client never sees this (we send straight to
        // upstream); its chat UI stays online.
        stanza = "<presence" + fromAttr + " type='unavailable'/>";
    } else {
        // Available + chosen show. For 'online' we use 'chat'. We send
        // BARE-available implicitly: a <presence> with <show> is treated
        // by XMPP servers as "available with show=X". Sending a bare
        // <presence/> first isn't needed because <presence><show>X</show>
        // means "available, X show".
        QString show = modeToString(m);
        stanza = "<presence" + fromAttr + ">"
                 "<show>" + show.toUtf8() + "</show>"
                 "</presence>";
    }

    // Bypass rewriter for our synth stanzas — we already built them with
    // the intended semantics. Run through anyway only to keep counters in
    // sync; the rewriter will see show=X (allowed) or unavailable (passthrough
    // — Offline path strips presence broadcasts, but this one has type=
    // attribute which our streaming code doesn't filter specifically).
    int sent = 0;
    for (auto it = d->byClient.begin(); it != d->byClient.end(); ++it) {
        auto *c = it.value();
        if (c->upstream && c->upstreamReady) {
            c->upstream->write(stanza);
            ++sent;
        }
    }
    emit log(QString("presence: pushed %1 (jid=%2) to %3 upstream(s)")
                 .arg(m == Mode::Offline ? "unavailable" : modeToString(m),
                      d->userJid.isEmpty() ? "?" : d->userJid).arg(sent));
}

} // namespace nyx
