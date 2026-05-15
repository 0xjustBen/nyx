#pragma once
#include <QByteArray>
#include <QString>
#include <vector>

namespace nyx {

enum class Mode {
    Online,     // "chat" — passthrough
    Away,       // "away"
    Mobile,     // "mobile" — strip game-in-progress info
    Dnd,        // "dnd"
    Offline,    // "offline"
};

Mode modeFromString(QString s);
QString modeToString(Mode m);

// Buffer-and-rewrite XMPP transform. Per Deceive's algorithm:
//   - Look for "<presence" substring in chunk
//   - Wrap in synthetic <xml>...</xml> root, parse with QDomDocument
//   - For each <presence> child:
//       * Drop MUC presence (has 'to' attr) unless connectToMuc
//       * Rewrite <show> and games/league_of_legends/st to target status
//       * Strip <status>, other-game extensions, etc per mode
//   - Serialize back, return bytes to forward
//
// If the chunk has no <presence stanza it's returned verbatim (zero-copy).
class XmppRewriter {
public:
    XmppRewriter();

    void setMode(Mode m);
    Mode mode() const { return m_mode; }

    void setConnectToMuc(bool v) { m_muc = v; }

    // Apply transform for client->server traffic. Returns possibly-modified bytes.
    QByteArray rewriteC2S(const QByteArray &chunk);

    // Server-to-client chunks are forwarded verbatim, but we parse them in
    // place to populate the friend roster + presence cache. The parsed data
    // is exposed via observeS2C and returned to the caller unchanged.
    struct Friend {
        QString jid;
        QString name;
        QString tag;
        QString presence;   // chat | away | dnd | mobile | unavailable
        QString game;       // LOL | VAL | LOR | 2XKO | ""
        QString activity;   // free-form status
    };
    void observeS2C(const QByteArray &chunk);

    // Pull pending roster events accumulated since last drain.
    struct Event {
        enum Kind { RosterItem, PresenceUpdate, Remove };
        Kind kind;
        Friend f;
    };
    std::vector<Event> drainEvents();

private:
    Mode m_mode = Mode::Online;
    bool m_muc = false;
    std::vector<Event> m_pending;
};

} // namespace nyx
