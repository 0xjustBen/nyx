#include "core/xmpp.hpp"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QStringList>
#include <QTextStream>

namespace nyx {

Mode modeFromString(QString s)
{
    s = s.toLower();
    if (s == "online" || s == "chat") return Mode::Online;
    if (s == "away")    return Mode::Away;
    if (s == "mobile")  return Mode::Mobile;
    if (s == "dnd")     return Mode::Dnd;
    if (s == "offline" || s == "invisible") return Mode::Offline;
    return Mode::Online;
}

QString modeToString(Mode m)
{
    switch (m) {
        case Mode::Online:  return "chat";
        case Mode::Away:    return "away";
        case Mode::Mobile:  return "mobile";
        case Mode::Dnd:     return "dnd";
        case Mode::Offline: return "offline";
    }
    return "chat";
}

XmppRewriter::XmppRewriter() = default;

void XmppRewriter::setMode(Mode m) { m_mode = m; }

void XmppRewriter::reset()
{
    m_c2sBuf.clear();
    m_pending.clear();
}

// Find end of the *first* top-level XMPP stanza starting at `start` in `data`.
// Returns index *after* the closing tag, or -1 if incomplete. Skips XML
// declarations and stream openers/closers (returns their boundary).
static int findStanzaEnd(const QByteArray &data, int start)
{
    while (start < data.size() && (data[start] == ' ' || data[start] == '\n' ||
                                    data[start] == '\r' || data[start] == '\t'))
        ++start;
    if (start >= data.size()) return -1;
    if (data[start] != '<') return -1;

    // <? ... ?> or <! ... > or <foo .../> or <foo>...</foo>
    if (data.mid(start, 2) == "<?") {
        int e = data.indexOf("?>", start);
        return e < 0 ? -1 : e + 2;
    }
    int nameEnd = start + 1;
    while (nameEnd < data.size() &&
           data[nameEnd] != ' ' && data[nameEnd] != '>' && data[nameEnd] != '/' &&
           data[nameEnd] != '\t' && data[nameEnd] != '\n' && data[nameEnd] != '\r')
        ++nameEnd;
    QByteArray name = data.mid(start + 1, nameEnd - start - 1);

    // Track depth from this stanza only.
    int i = start;
    int depth = 0;
    while (i < data.size()) {
        if (data[i] != '<') { ++i; continue; }
        // Self-closed <foo .../>
        int gt = data.indexOf('>', i);
        if (gt < 0) return -1;
        bool selfClosed = (gt > 0 && data[gt - 1] == '/');
        bool isClose = (i + 1 < data.size() && data[i + 1] == '/');

        if (selfClosed && depth == 0) return gt + 1;
        if (isClose) {
            --depth;
            if (depth == 0) return gt + 1;
        } else if (!selfClosed) {
            ++depth;
        } else {
            // self-closed nested element, no depth change
        }
        i = gt + 1;
    }
    return -1; // incomplete
}

QByteArray XmppRewriter::rewriteC2S(const QByteArray &chunk)
{
    // Fast path: passthrough if Online.
    if (m_mode == Mode::Online) return chunk;

    m_c2sBuf.append(chunk);

    QByteArray out;
    int i = 0;
    while (i < m_c2sBuf.size()) {
        // Skip whitespace.
        while (i < m_c2sBuf.size() && (m_c2sBuf[i] == ' ' || m_c2sBuf[i] == '\n' ||
                                       m_c2sBuf[i] == '\r' || m_c2sBuf[i] == '\t')) {
            out.append(m_c2sBuf[i]);
            ++i;
        }
        if (i >= m_c2sBuf.size()) break;
        int end = findStanzaEnd(m_c2sBuf, i);
        if (end < 0) break; // incomplete; keep in buffer
        QByteArray stanza = m_c2sBuf.mid(i, end - i);
        // Only mutate <presence> stanzas via legacy single-stanza path.
        if (stanza.contains("<presence")) {
            QByteArray rewritten = rewriteSingleStanza(stanza);
            out.append(rewritten);
        } else {
            out.append(stanza);
        }
        i = end;
    }
    m_c2sBuf.remove(0, i);
    return out;
}

// Original single-stanza rewriter — extracted from rewriteC2S.
QByteArray XmppRewriter::rewriteSingleStanza(const QByteArray &chunk)
{
    if (!chunk.contains("<presence")) return chunk;

    // INVISIBLE / OFFLINE: drop outbound broadcast <presence> entirely.
    // Server never sees a fresh presence so it doesn't broadcast to our
    // roster. Friends keep seeing the last-known state (typically offline
    // for a session that started in invisible). Crucially the Riot Client
    // still believes IT is online (no server-side echo with show=offline)
    // so the local chat UI stays fully functional — user can send/receive
    // DMs while appearing offline to everyone else.
    if (m_mode == Mode::Offline) {
        // Strip every top-level <presence ...> ... </presence> (or self-closed).
        QByteArray out;
        int i = 0;
        while (i < chunk.size()) {
            int p = chunk.indexOf("<presence", i);
            if (p < 0) { out.append(chunk.mid(i)); break; }
            out.append(chunk.mid(i, p - i));
            // Find matching close.
            int gt = chunk.indexOf('>', p);
            if (gt < 0) break;
            bool selfClosed = (gt > 0 && chunk[gt - 1] == '/');
            if (selfClosed) { i = gt + 1; continue; }
            int close = chunk.indexOf("</presence>", gt);
            if (close < 0) break;
            i = close + (int)strlen("</presence>");
        }
        return out;
    }

    // Wrap in synthetic root so QDomDocument can parse a stanza-fragment chunk
    // that may contain multiple top-level <presence> stanzas.
    QByteArray wrapped = "<xml>" + chunk + "</xml>";
    QDomDocument doc;
    if (!doc.setContent(wrapped, /*namespaceProcessing*/ false)) {
        return chunk; // give up on this chunk; safer than corrupting stream
    }
    QDomElement root = doc.documentElement();
    if (root.isNull()) return chunk;

    const QString target = modeToString(m_mode);

    QDomElement el = root.firstChildElement();
    while (!el.isNull()) {
        QDomElement next = el.nextSiblingElement();
        if (el.tagName() != "presence") { el = next; continue; }

        // MUC presence (has 'to' attribute) — drop unless explicitly enabled.
        if (el.hasAttribute("to")) {
            if (!m_muc) {
                root.removeChild(el);
                el = next;
                continue;
            }
        }

        auto setTextOrCreate = [](QDomElement parent, const QString &tag, const QString &val) {
            QDomElement e = parent.firstChildElement(tag);
            if (e.isNull()) return;
            while (e.hasChildNodes()) e.removeChild(e.firstChild());
            e.appendChild(parent.ownerDocument().createTextNode(val));
        };

        QDomElement games = el.firstChildElement("games");
        QDomElement lol   = games.isNull() ? QDomElement() : games.firstChildElement("league_of_legends");
        QDomElement st    = lol.isNull()   ? QDomElement() : lol.firstChildElement("st");

        // Per Deceive: skip <show>/st rewrite if user is targeting dnd AND lol/st already dnd
        // (means the in-game dnd flag is already set, leave it).
        if (!(target != "chat" && !st.isNull() && st.text() == "dnd")) {
            setTextOrCreate(el, "show", target);
            if (!st.isNull()) setTextOrCreate(lol, "st", target);
        }

        if (target == "chat") { el = next; continue; }

        // Strip status text (presence-detail string visible to friends).
        QDomElement status = el.firstChildElement("status");
        if (!status.isNull()) el.removeChild(status);

        if (!games.isNull()) {
            if (target == "mobile") {
                // Keep lol element but remove game-in-progress info.
                if (!lol.isNull()) {
                    QDomElement p = lol.firstChildElement("p");
                    if (!p.isNull()) lol.removeChild(p);
                    QDomElement m = lol.firstChildElement("m");
                    if (!m.isNull()) lol.removeChild(m);
                }
            } else {
                if (!lol.isNull()) games.removeChild(lol);
            }
            // Always strip other-game presence so friends don't see e.g. LoR/2XKO/etc.
            for (const QString &tag : {"bacon", "lion", "keystone", "riot_client"}) {
                QDomElement g = games.firstChildElement(tag);
                if (!g.isNull()) games.removeChild(g);
            }
        }

        el = next;
    }

    // Serialize root children back without the synthetic <xml> wrapper.
    QByteArray out;
    QDomNode n = root.firstChild();
    while (!n.isNull()) {
        QString s;
        QTextStream ts(&s);
        n.save(ts, /*indent*/ 0);
        out += s.toUtf8();
        n = n.nextSibling();
    }
    return out;
}

// Parse S2C bytes for roster + presence events. Best-effort — wraps and
// scans, doesn't fail on partial chunks.
void XmppRewriter::observeS2C(const QByteArray &chunk)
{
    if (!chunk.contains("<presence") && !chunk.contains("<item")) return;

    QByteArray wrapped = "<xml>" + chunk + "</xml>";
    QDomDocument doc;
    if (!doc.setContent(wrapped, /*namespaceProcessing*/ false)) return;
    QDomElement root = doc.documentElement();
    if (root.isNull()) return;

    auto deriveGame = [](const QDomElement &games) -> QString {
        if (games.isNull()) return {};
        if (!games.firstChildElement("league_of_legends").isNull()) return "LOL";
        if (!games.firstChildElement("valorant").isNull())           return "VAL";
        if (!games.firstChildElement("bacon").isNull())              return "LOR";
        if (!games.firstChildElement("lion").isNull())               return "2XKO";
        return {};
    };
    auto splitJid = [](const QString &full) {
        // bare@host/resource → bare and tag
        int slash = full.indexOf('/');
        QString bare = slash < 0 ? full : full.left(slash);
        return bare;
    };

    // Roster IQ items.
    QDomNodeList items = root.elementsByTagName("item");
    for (int i = 0; i < items.size(); ++i) {
        QDomElement it = items.at(i).toElement();
        if (it.isNull()) continue;
        // Riot's roster items have name + puuid attributes.
        QString jid  = it.attribute("jid");
        QString name = it.attribute("name");
        if (jid.isEmpty()) continue;

        Friend f;
        f.jid = splitJid(jid);
        f.name = name.isEmpty() ? f.jid.section('@', 0, 0) : name;
        // Tag = guess from JID host (e.g. eu1.pvp.net → EU1)
        QString host = f.jid.section('@', 1).section('.', 0, 0).toUpper();
        if (!host.isEmpty()) f.tag = "#" + host;
        f.presence = "offline";
        m_pending.push_back({Event::RosterItem, f});
    }

    // Presence updates.
    QDomElement el = root.firstChildElement();
    while (!el.isNull()) {
        if (el.tagName() == "presence") {
            QString from = el.attribute("from");
            if (!from.isEmpty()) {
                Friend f;
                f.jid = from.section('/', 0, 0);
                QDomElement show = el.firstChildElement("show");
                QDomElement games = el.firstChildElement("games");
                QDomElement status = el.firstChildElement("status");
                QString type = el.attribute("type");
                if (type == "unavailable") {
                    f.presence = "offline";
                } else if (!show.isNull()) {
                    f.presence = show.text();
                } else {
                    f.presence = "chat";
                }
                f.game = deriveGame(games);
                if (!status.isNull()) f.activity = status.text();
                m_pending.push_back({Event::PresenceUpdate, f});
            }
        }
        el = el.nextSiblingElement();
    }
}

std::vector<XmppRewriter::Event> XmppRewriter::drainEvents()
{
    std::vector<Event> out;
    out.swap(m_pending);
    return out;
}

} // namespace nyx
