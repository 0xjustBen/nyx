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

    // Apply transform for client->server traffic. Stream-aware: buffers
    // partial stanzas across TCP chunks and only emits bytes for complete
    // top-level stanzas (so QDomDocument parse never sees a split tag).
    QByteArray rewriteC2S(const QByteArray &chunk);

    // Reset stream buffers (call on new connection).
    void reset();

    // Server-to-client chunks: parse to populate roster + presence cache.
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

    // Internal — exposed for the streaming rewriter to call on each complete stanza.
    QByteArray rewriteSingleStanza(const QByteArray &chunk);

private:
    Mode m_mode = Mode::Online;
    bool m_muc = false;
    std::vector<Event> m_pending;
    QByteArray m_c2sBuf; // stream buffer
};

} // namespace nyx
