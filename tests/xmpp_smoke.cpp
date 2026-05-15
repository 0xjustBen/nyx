// XMPP rewriter smoke. Run each mode against representative presence stanza.

#include "core/xmpp.hpp"

#include <QCoreApplication>
#include <QByteArray>
#include <QString>
#include <QDebug>

#include <cstdio>
#include <cstdlib>

using nyx::XmppRewriter;
using nyx::Mode;

static const QByteArray kPresence = QByteArrayLiteral(
    "<presence>"
      "<games>"
        "<league_of_legends>"
          "<st>chat</st>"
          "<s.t>10000</s.t>"
          "<m>{some-match-json}</m>"
          "<p>{some-progress}</p>"
        "</league_of_legends>"
        "<keystone><st>chat</st></keystone>"
        "<bacon><st>chat</st></bacon>"
        "<lion><st>chat</st></lion>"
        "<riot_client><st>chat</st></riot_client>"
      "</games>"
      "<show>chat</show>"
      "<status>some status text</status>"
    "</presence>"
);

static int fail = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::fprintf(stderr, "FAIL: %s\n", msg); ++fail; } } while (0)

static QByteArray run(Mode m, const QByteArray &in)
{
    XmppRewriter r;
    r.setMode(m);
    return r.rewriteC2S(in);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Online: passthrough.
    {
        QByteArray out = run(Mode::Online, kPresence);
        CHECK(out == kPresence, "online passthrough");
    }

    // Away: show=away, lol removed, other-games stripped, status gone.
    {
        QByteArray out = run(Mode::Away, kPresence);
        CHECK(out.contains("<show>away</show>"), "away: show rewritten");
        CHECK(!out.contains("<status>"),         "away: status removed");
        CHECK(!out.contains("<league_of_legends>"), "away: lol removed");
        CHECK(!out.contains("<bacon>") && !out.contains("<lion>") &&
              !out.contains("<keystone>") && !out.contains("<riot_client>"),
              "away: other games removed");
    }

    // Mobile: show=mobile, lol KEPT but p/m stripped.
    {
        QByteArray out = run(Mode::Mobile, kPresence);
        CHECK(out.contains("<show>mobile</show>"),   "mobile: show");
        CHECK(out.contains("<league_of_legends>"),   "mobile: lol kept");
        CHECK(!out.contains("<p>"),                  "mobile: p stripped");
        CHECK(!out.contains("<m>"),                  "mobile: m stripped");
        CHECK(!out.contains("<bacon>"),              "mobile: other-game stripped");
    }

    // Offline (invisible): outbound presence dropped entirely so server
    // never broadcasts a fresh state. Friends see last-known (typically
    // offline) and the client itself stays online.
    {
        QByteArray out = run(Mode::Offline, kPresence);
        CHECK(out.trimmed().isEmpty() || !out.contains("<presence"),
              "offline: outbound presence dropped");
    }

    // Dnd: show=dnd, lol removed.
    {
        QByteArray out = run(Mode::Dnd, kPresence);
        CHECK(out.contains("<show>dnd</show>"),      "dnd: show");
    }

    // MUC presence (has 'to' attr) — should be dropped in non-MUC mode.
    {
        QByteArray muc = "<presence to='muc@room/jid'><show>chat</show></presence>";
        QByteArray out = run(Mode::Away, muc);
        CHECK(out.trimmed().isEmpty() || !out.contains("<presence"), "MUC dropped");
    }

    // Non-presence chunk: passthrough.
    {
        QByteArray msg = "<message from='x' to='y'><body>hi</body></message>";
        QByteArray out = run(Mode::Away, msg);
        CHECK(out == msg, "non-presence passthrough");
    }

    if (fail == 0) { std::printf("ok (%d cases)\n", 8); return 0; }
    std::fprintf(stderr, "%d failure(s)\n", fail);
    return 1;
}
