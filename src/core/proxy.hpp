#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace nyx {

class ProxyService {
public:
    using StanzaCallback = std::function<void(const std::string &kind)>;
    using ConnectedCallback = std::function<void(bool)>;

    ProxyService();
    ~ProxyService();

    bool start(uint16_t listen_port);
    void stop();

    void setMode(const std::string &mode);
    std::string mode() const;

    void onStanza(StanzaCallback cb);
    void onConnected(ConnectedCallback cb);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace nyx
