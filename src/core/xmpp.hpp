#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>

namespace nyx {

enum class Mode {
    Online,
    Away,
    Mobile,
    Offline,
    Invisible,
};

Mode modeFromString(std::string_view s);
const char *modeToString(Mode m);

// Streaming XMPP stanza rewriter. Feed chunks of bytes from either direction;
// callback fires with possibly-rewritten bytes to forward.
class XmppRewriter {
public:
    using EmitFn = std::function<void(const char *data, size_t n)>;

    XmppRewriter();
    ~XmppRewriter();

    void setMode(Mode m);
    Mode mode() const;

    // c2s = client -> server, s2c = server -> client.
    void feedC2S(const char *data, size_t n, const EmitFn &emit);
    void feedS2C(const char *data, size_t n, const EmitFn &emit);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace nyx
