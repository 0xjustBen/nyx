#include "core/xmpp.hpp"

#include <string>
#include <atomic>

namespace nyx {

Mode modeFromString(std::string_view s)
{
    if (s == "online")    return Mode::Online;
    if (s == "away")      return Mode::Away;
    if (s == "mobile")    return Mode::Mobile;
    if (s == "offline")   return Mode::Offline;
    if (s == "invisible") return Mode::Invisible;
    return Mode::Online;
}

const char *modeToString(Mode m)
{
    switch (m) {
        case Mode::Online:    return "online";
        case Mode::Away:      return "away";
        case Mode::Mobile:    return "mobile";
        case Mode::Offline:   return "offline";
        case Mode::Invisible: return "invisible";
    }
    return "online";
}

struct XmppRewriter::Impl {
    std::atomic<Mode> mode{Mode::Online};
    std::string c2sBuf;
    std::string s2cBuf;
};

XmppRewriter::XmppRewriter() : d(std::make_unique<Impl>()) {}
XmppRewriter::~XmppRewriter() = default;

void XmppRewriter::setMode(Mode m) { d->mode.store(m); }
Mode XmppRewriter::mode() const { return d->mode.load(); }

// Stub: forward verbatim. Real impl uses streaming XML parser (pugixml or
// hand-rolled) to detect <presence> stanzas and rewrite <show>/<status>,
// strip <league_of_legends> game extension, and (for Offline mode) suppress
// outbound presence broadcasts entirely.
void XmppRewriter::feedC2S(const char *data, size_t n, const EmitFn &emit)
{
    if (emit) emit(data, n);
}

void XmppRewriter::feedS2C(const char *data, size_t n, const EmitFn &emit)
{
    if (emit) emit(data, n);
}

} // namespace nyx
