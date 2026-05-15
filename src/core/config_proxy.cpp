#include "core/config_proxy.hpp"

#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPointer>
#include <QTimer>
#include <QFile>

namespace nyx {

namespace {

constexpr const char *kConfigUrl = "https://clientconfig.rpg.riotgames.com";

// Tiny HTTP/1.1 request parse (request line + headers, no body — clientconfig
// is GET-only from Riot Client). Body intentionally ignored.
struct HttpReq {
    QByteArray method;
    QByteArray path;
    QList<QPair<QByteArray, QByteArray>> headers;
    bool complete = false;
};

bool parseRequest(const QByteArray &buf, HttpReq &out)
{
    int hdrEnd = buf.indexOf("\r\n\r\n");
    if (hdrEnd < 0) return false;
    QByteArray head = buf.left(hdrEnd);
    auto lines = head.split('\n');
    if (lines.isEmpty()) return false;
    auto reqLine = lines[0];
    if (reqLine.endsWith('\r')) reqLine.chop(1);
    auto parts = reqLine.split(' ');
    if (parts.size() < 3) return false;
    out.method = parts[0];
    out.path = parts[1];
    for (int i = 1; i < lines.size(); ++i) {
        QByteArray l = lines[i];
        if (l.endsWith('\r')) l.chop(1);
        if (l.isEmpty()) continue;
        int c = l.indexOf(':');
        if (c <= 0) continue;
        QByteArray k = l.left(c).trimmed();
        QByteArray v = l.mid(c + 1).trimmed();
        out.headers.append({k, v});
    }
    out.complete = true;
    return true;
}

QByteArray header(const HttpReq &r, const QByteArray &name)
{
    for (const auto &p : r.headers)
        if (p.first.compare(name, Qt::CaseInsensitive) == 0) return p.second;
    return {};
}

const char *reasonPhrase(int status)
{
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        case 502: return "Bad Gateway";
        case 503: return "Service Unavailable";
        default:  return "OK";
    }
}

QByteArray buildResponse(int status, const QByteArray &body)
{
    QByteArray r;
    r += "HTTP/1.1 " + QByteArray::number(status) + " " + reasonPhrase(status) + "\r\n";
    r += "Content-Type: application/json\r\n";
    r += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
    r += "Connection: close\r\n\r\n";
    r += body;
    return r;
}

} // namespace

struct ConfigProxy::Impl {
    QTcpServer *server = nullptr;
    QNetworkAccessManager *http = nullptr;
    QString localhostDomain;
    uint16_t chatPort = 0;
    uint16_t listenPort = 0;
    bool chatResolvedEmitted = false;
};

ConfigProxy::ConfigProxy(QObject *parent) : QObject(parent), d(std::make_unique<Impl>()) {}
ConfigProxy::~ConfigProxy() { stop(); }

bool ConfigProxy::start(const QString &localhostDomain, uint16_t chatProxyPort)
{
    if (d->server) return false;
    d->localhostDomain = localhostDomain;
    d->chatPort = chatProxyPort;
    d->chatResolvedEmitted = false;
    d->http = new QNetworkAccessManager(this);
    d->server = new QTcpServer(this);

    connect(d->server, &QTcpServer::newConnection, this, [this] {
        while (QTcpSocket *sock = d->server->nextPendingConnection()) {
            auto *buf = new QByteArray();
            auto *inflight = new bool(false);

            auto cleanup = [buf, inflight, sock] {
                delete buf;
                delete inflight;
                sock->deleteLater();
            };
            connect(sock, &QTcpSocket::disconnected, this, cleanup);

            connect(sock, &QTcpSocket::readyRead, this, [this, sock, buf, inflight] {
                if (*inflight) {
                    // Already proxying an upstream request on this connection;
                    // ignore further pipelined bytes until reply arrives.
                    buf->append(sock->readAll());
                    return;
                }
                buf->append(sock->readAll());
                HttpReq req;
                if (!parseRequest(*buf, req)) return;
                *inflight = true;

                emit log("configproxy → upstream " + QString::fromLatin1(req.method) + " "
                         + QString::fromLatin1(req.path));
                QUrl url(QString::fromLatin1(kConfigUrl) + QString::fromLatin1(req.path));
                QNetworkRequest qreq(url);
                auto ua = header(req, "User-Agent");
                if (!ua.isEmpty()) qreq.setRawHeader("User-Agent", ua);
                auto jwt = header(req, "X-Riot-Entitlements-JWT");
                if (!jwt.isEmpty()) qreq.setRawHeader("X-Riot-Entitlements-JWT", jwt);
                auto authz = header(req, "Authorization");
                if (!authz.isEmpty()) qreq.setRawHeader("Authorization", authz);

                QPointer<QTcpSocket> sockGuard(sock);
                QNetworkReply *reply = d->http->get(qreq);
                connect(reply, &QNetworkReply::finished, this,
                        [this, sockGuard, buf, inflight, reply] {
                    QByteArray body = reply->readAll();
                    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    if (status == 0) status = 502;

                    // Log path + size for every proxied request.
                    emit log(QString("configproxy ← upstream %1 bytes (status %2)")
                                 .arg(body.size()).arg(status));
                    if (status >= 200 && status < 300) {
                        QJsonDocument doc = QJsonDocument::fromJson(body);
                        if (doc.isObject()) {
                            QJsonObject obj = doc.object();
                            QString realHost;
                            int realPort = 0;
                            if (obj.contains("chat.host")) {
                                realHost = obj.value("chat.host").toString();
                                obj["chat.host"] = d->localhostDomain;
                            }
                            if (obj.contains("chat.port")) {
                                realPort = obj.value("chat.port").toInt();
                                obj["chat.port"] = (int)d->chatPort;
                            }
                            if (obj.contains("chat.affinities") && obj.value("chat.affinities").isObject()) {
                                QJsonObject aff = obj.value("chat.affinities").toObject();
                                for (auto it = aff.begin(); it != aff.end(); ++it) {
                                    it.value() = d->localhostDomain;
                                }
                                obj["chat.affinities"] = aff;
                            }
                            body = QJsonDocument(obj).toJson(QJsonDocument::Compact);

                            // Optional debug dump.
                            if (qEnvironmentVariableIsSet("NYX_DUMP_CFG")) {
                                QFile f(qEnvironmentVariable("NYX_DUMP_CFG"));
                                if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
                                    f.write("---\n");
                                    f.write(body);
                                    f.write("\n");
                                }
                            }

                            if (!d->chatResolvedEmitted && !realHost.isEmpty() && realPort > 0) {
                                d->chatResolvedEmitted = true;
                                emit log(QString("chat resolved: %1:%2").arg(realHost).arg(realPort));
                                emit chatServerResolved(realHost, (uint16_t)realPort);
                            }
                        }
                    }
                    if (sockGuard && sockGuard->state() == QAbstractSocket::ConnectedState) {
                        sockGuard->write(buildResponse(status, body));
                        sockGuard->disconnectFromHost();
                    }
                    buf->clear();
                    *inflight = false;
                    reply->deleteLater();
                });
            });
        }
    });

    if (!d->server->listen(QHostAddress::LocalHost, 0)) {
        emit log("configproxy: listen failed: " + d->server->errorString());
        return false;
    }
    d->listenPort = (uint16_t)d->server->serverPort();
    emit log(QString("configproxy: listening on http://127.0.0.1:%1").arg(d->listenPort));
    return true;
}

void ConfigProxy::stop()
{
    if (d->server) { d->server->close(); d->server->deleteLater(); d->server = nullptr; }
    if (d->http)   { d->http->deleteLater();   d->http = nullptr; }
}

uint16_t ConfigProxy::port() const { return d->listenPort; }
void ConfigProxy::setChatPort(uint16_t p) { d->chatPort = p; }

} // namespace nyx
