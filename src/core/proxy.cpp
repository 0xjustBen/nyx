#include "core/proxy.hpp"

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

            connect(sock, &QSslSocket::encrypted, this, [this, c] {
                emit log("client: TLS up — dialing upstream " + d->upstreamHost);
                c->upstream = new QSslSocket(this);
                d->byUpstream.insert(c->upstream, c);
                c->upstream->setProtocol(QSsl::TlsV1_2OrLater);
                c->upstream->setPeerVerifyMode(QSslSocket::VerifyNone); // TODO: pin Riot's CA
                connect(c->upstream, &QSslSocket::encrypted, this, [this, c] {
                    c->upstreamReady = true;
                    emit log("upstream: TLS up");
                    if (!c->pending.isEmpty()) {
                        c->upstream->write(c->pending);
                        d->c2sBytes += c->pending.size();
                        c->pending.clear();
                    }
                });
                connect(c->upstream, &QSslSocket::readyRead, this, [this, c] {
                    QByteArray b = c->upstream->readAll();
                    d->s2cBytes += b.size();
                    if (c->client && c->client->state() == QAbstractSocket::ConnectedState)
                        c->client->write(b);
                    emit bytesPumped(d->c2sBytes, d->s2cBytes);
                });
                connect(c->upstream, &QSslSocket::disconnected, this, [this, c] {
                    emit log("upstream: disconnected");
                    if (c->client) c->client->disconnectFromHost();
                });
                connect(c->upstream, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
                        this, [this, c](const QList<QSslError> &errs) {
                    for (const auto &e : errs) emit log("upstream ssl: " + e.errorString());
                    c->upstream->ignoreSslErrors(); // dev only
                });
                c->upstream->connectToHostEncrypted(d->upstreamHost, d->upstreamPort);
            });
            connect(sock, &QSslSocket::readyRead, this, [this, c] {
                QByteArray b = c->client->readAll();
                if (c->upstreamReady) {
                    c->upstream->write(b);
                    d->c2sBytes += b.size();
                    emit bytesPumped(d->c2sBytes, d->s2cBytes);
                } else {
                    c->pending.append(b);
                }
            });
            connect(sock, &QSslSocket::disconnected, this, [this, c] {
                emit log("client: disconnected");
                emit clientDisconnected();
                if (c->upstream) c->upstream->disconnectFromHost();
                d->byClient.remove(c->client);
                if (c->upstream) d->byUpstream.remove(c->upstream);
                c->client->deleteLater();
                if (c->upstream) c->upstream->deleteLater();
                delete c;
            });
            connect(sock, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
                    this, [this](const QList<QSslError> &errs) {
                for (const auto &e : errs) emit log("client ssl: " + e.errorString());
            });
        }
    });

    if (!d->server->listen(QHostAddress::LocalHost, listenPort)) {
        emit log(QString("listen failed on %1: %2").arg(listenPort).arg(d->server->errorString()));
        d->server->deleteLater();
        d->server = nullptr;
        return false;
    }
    emit log(QString("listening on 127.0.0.1:%1, upstream %2:%3")
                 .arg(listenPort).arg(d->upstreamHost).arg(d->upstreamPort));
    return true;
}

void ProxyService::stop()
{
    if (!d->server) return;
    d->server->close();
    d->server->deleteLater();
    d->server = nullptr;
}

void ProxyService::setMode(const QString &m) { d->mode = m; }
QString ProxyService::mode() const { return d->mode; }

} // namespace nyx
