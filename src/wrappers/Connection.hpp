#pragma once

#include <memory>

#include <DXFeed.h>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "ConnectionStatus.hpp"
#include "helpers/Handler.hpp"

namespace dxfcs {

struct Connection final : public std::enable_shared_from_this<Connection> {
    Connection &operator=(Connection &) = delete;

  private:
    dxf_connection_t connectionHandle_ = nullptr;

    Handler<void()> onDisconnect_{};
    Handler<void(const ConnectionStatus &, const ConnectionStatus &)> onConnectionStatusChanged_{};

    template <typename F = std::function<void(std::shared_ptr<Connection> &)>>
    static std::shared_ptr<Connection> create(const std::string &address, F &&beforeConnect) {
        auto c = std::make_shared<Connection>();

        beforeConnect(c);

        dxf_connection_t connectionHandle = nullptr;
        auto r = dxf_create_connection(
            address.c_str(),
            [](dxf_connection_t, void *data) { reinterpret_cast<Connection *>(data)->onDisconnect_(); },
            [](dxf_connection_t, dxf_connection_status_t oldStatus, dxf_connection_status_t newStatus, void *data) {
                reinterpret_cast<Connection *>(data)->onConnectionStatusChanged_(ConnectionStatus::get(oldStatus),
                                                                                 ConnectionStatus::get(newStatus));
            },
            nullptr, nullptr, reinterpret_cast<void *>(c.get()), &connectionHandle);

        if (r == DXF_FAILURE) {
            return {};
        }

        c->connectionHandle_ = connectionHandle;

        // TODO: logging
        return c;
    }

  public:
    ~Connection() {
        if (connectionHandle_ != nullptr) {
            dxf_close_connection(connectionHandle_);
        }
    }

    Handler<void()> &onDisconnect() { return onDisconnect_; }

    Handler<void(const ConnectionStatus &, const ConnectionStatus &)> &onConnectionStatusChanged() {
        return onConnectionStatusChanged_;
    }

    template <typename OnDisconnectListener = typename Handler<void()>::ListenerType,
              typename OnConnectionStatusChangedListener =
                  typename Handler<void(const ConnectionStatus &, const ConnectionStatus &)>::ListenerType>
    static std::shared_ptr<Connection> create(const std::string &address, OnDisconnectListener &&onDisconnectListener,
                                              OnConnectionStatusChangedListener &&onConnectionStatusChangedListener) {
        return create(address,
                      [&onDisconnectListener, &onConnectionStatusChangedListener](std::shared_ptr<Connection> &c) {
                          c->onDisconnect() += std::forward<OnDisconnectListener>(onDisconnectListener);
                          c->onConnectionStatusChanged() +=
                              std::forward<OnConnectionStatusChangedListener>(onConnectionStatusChangedListener);
                      });
    }

    static std::shared_ptr<Connection> create(const std::string &address) {
        return create(address, [](std::shared_ptr<Connection> &) {});
    }
};

} // namespace dxfcs

/*
{
    auto c = dxfcs::DXFeed::connect(
        "demo.dxfeed.com:7300", []() { std::cout << "Disconnected" << std::endl; },
        [](const dxfcs::ConnectionStatus &oldStatus, const dxfcs::ConnectionStatus &newStatus) {
            std::cout << "Status: " << oldStatus << " -> " << newStatus << std::endl;
        });

c->onConnectionStatusChanged() +=
[](const dxfcs::ConnectionStatus &oldStatus, const dxfcs::ConnectionStatus &newStatus) {
    std::cout << "Status 2: " << oldStatus << " -> " << newStatus << std::endl;
};

std::this_thread::sleep_for(std::chrono::seconds(5));
}
 */