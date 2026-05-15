#include "core/proxy.hpp"
#include "core/xmpp.hpp"

#include <mutex>
#include <thread>
#include <atomic>

namespace nyx {

struct ProxyService::Impl {
    std::atomic<bool> running{false};
    std::thread worker;
    std::string mode = "online";
    StanzaCallback stanzaCb;
    ConnectedCallback connectedCb;
    std::mutex cbMu;
};

ProxyService::ProxyService() : d(std::make_unique<Impl>()) {}
ProxyService::~ProxyService() { stop(); }

bool ProxyService::start(uint16_t /*listen_port*/)
{
    if (d->running.exchange(true)) return false;
    // TODO(phase-2): bind TLS listener on 127.0.0.1:listen_port using OpenSSL,
    // open upstream TLS to chat.<region>.lol.riotgames.com:5223,
    // pipe bytes through XmppRewriter with current mode.
    d->worker = std::thread([this]{
        while (d->running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    });
    return true;
}

void ProxyService::stop()
{
    if (!d->running.exchange(false)) return;
    if (d->worker.joinable()) d->worker.join();
}

void ProxyService::setMode(const std::string &m) { d->mode = m; }
std::string ProxyService::mode() const { return d->mode; }

void ProxyService::onStanza(StanzaCallback cb)
{
    std::lock_guard<std::mutex> g(d->cbMu);
    d->stanzaCb = std::move(cb);
}

void ProxyService::onConnected(ConnectedCallback cb)
{
    std::lock_guard<std::mutex> g(d->cbMu);
    d->connectedCb = std::move(cb);
}

} // namespace nyx
