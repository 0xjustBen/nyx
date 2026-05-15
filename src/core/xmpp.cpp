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

QByteArray XmppRewriter::rewriteC2S(const QByteArray &chunk)
{
    if (m_mode == Mode::Online) return chunk;
    if (!chunk.contains("<presence")) return chunk;

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

} // namespace nyx
