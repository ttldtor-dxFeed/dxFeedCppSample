#pragma once

#include <memory>

#include <DXFeed.h>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <vector>

#include "ConnectionStatus.hpp"
#include "helpers/Handler.hpp"

namespace dxfcs {

struct Connection final : public std::enable_shared_from_this<Connection> {
    Connection &operator=(Connection &) = delete;

  private:
    dxf_connection_t con = nullptr;

    Handler<void()> onDisconnect_{};
    Handler<void(const ConnectionStatus &, const ConnectionStatus &)> onConnectionStatusChanged_{};

  public:
    ~Connection() {
        if (con != nullptr) {
            dxf_close_connection(con);
        }
    }

    Handler<void()> &onDisconnect() { return onDisconnect_; }

    Handler<void(const ConnectionStatus &, const ConnectionStatus &)> &onConnectionStatusChanged() {
        return onConnectionStatusChanged_;
    }

    template <typename OnDisconnectListener = typename Handler<void()>::ListenerType,
              typename OnConnectionStatusChangedListener =
                  typename Handler<void(const ConnectionStatus &, const ConnectionStatus &)>::ListenerType>
    static std::shared_ptr<Connection> create(const std::string &address, OnDisconnectListener &&disconnectListener,
                                              OnConnectionStatusChangedListener &&connectionStatusChangedListener) {
        auto c = std::make_shared<Connection>();

        c->onDisconnect() += std::forward<OnDisconnectListener>(disconnectListener);
        c->onConnectionStatusChanged() +=
            std::forward<OnConnectionStatusChangedListener>(connectionStatusChangedListener);

        dxf_connection_t con = nullptr;
        auto r = dxf_create_connection(
            address.c_str(),
            [](dxf_connection_t, void *data) {
                reinterpret_cast<Connection *>(data)->shared_from_this()->onDisconnect_();
            },
            [](dxf_connection_t, dxf_connection_status_t oldStatus, dxf_connection_status_t newStatus, void *data) {
                reinterpret_cast<Connection *>(data)->onConnectionStatusChanged_(ConnectionStatus::get(oldStatus),
                                                                                 ConnectionStatus::get(newStatus));
            },
            nullptr, nullptr, reinterpret_cast<void *>(c.get()), &con);

        if (r == DXF_FAILURE) {
            return {};
        }

        // TODO: implement handlers, logging, callbacks
        return c;
    }

    static std::shared_ptr<Connection> create(const std::string &address) {
        auto c = std::make_shared<Connection>();

        dxf_connection_t con = nullptr;
        auto r = dxf_create_connection(
            address.c_str(),
            [](dxf_connection_t, void *data) {
                reinterpret_cast<Connection *>(data)->shared_from_this()->onDisconnect_();
            },
            [](dxf_connection_t, dxf_connection_status_t oldStatus, dxf_connection_status_t newStatus, void *data) {
                reinterpret_cast<Connection *>(data)->onConnectionStatusChanged_(ConnectionStatus::get(oldStatus),
                                                                                 ConnectionStatus::get(newStatus));
            },
            nullptr, nullptr, reinterpret_cast<void *>(c.get()), &con);

        if (r == DXF_FAILURE) {
            return {};
        }

        // TODO: implement handlers, logging, callbacks
        return c;
    }
};

} // namespace dxfcs

/*
{
auto c = dxfcs::Connection::create(
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