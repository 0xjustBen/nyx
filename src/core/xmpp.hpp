#pragma once
#include <QByteArray>
#include <QString>

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

private:
    Mode m_mode = Mode::Online;
    bool m_muc = false;
};

} // namespace nyx
