#pragma once

#include <memory>

#include <DXFeed.h>

#include <functional>
#include <mutex>
#include <string>
#include <utility>

#include "DXFCppConfig.hpp"

#include "ConnectionStatus.hpp"
#include "helpers/Handler.hpp"

#include "Subscription.hpp"

namespace dxfcpp {

struct Subscription;

struct Connection final : public std::enable_shared_from_this<Connection> {
    using Ptr = std::shared_ptr<Connection>;

    static const Ptr INVALID;

  private:

    mutable std::recursive_mutex mutex_{};
    dxf_connection_t connectionHandle_ = nullptr;

    Handler<void()> onDisconnect_{};
    Handler<void(ConnectionStatus, ConnectionStatus)> onConnectionStatusChanged_{};

    std::vector<Subscription::Ptr> subscriptions_{};

    template <typename F = std::function<void(Ptr &)>>
    static Ptr createImpl(const std::string &address, F &&beforeConnect) {
        auto c = std::make_shared<Connection>();

        beforeConnect(c);

        dxf_connection_t connectionHandle = nullptr;
        auto r = dxf_create_connection(
            address.c_str(),
            [](dxf_connection_t, void *userData) { reinterpret_cast<Connection *>(userData)->onDisconnect_(); },
            [](dxf_connection_t, dxf_connection_status_t oldStatus, dxf_connection_status_t newStatus, void *userData) {
                reinterpret_cast<Connection *>(userData)->onConnectionStatusChanged_(ConnectionStatus::get(oldStatus),
                                                                                     ConnectionStatus::get(newStatus));
            },
            nullptr, nullptr, reinterpret_cast<void *>(c.get()), &connectionHandle);

        if (r == DXF_FAILURE) {
            return INVALID;
        }

        c->connectionHandle_ = connectionHandle;

        // TODO: logging
        return c;
    }

  public:

    Connection &operator=(Connection &) = delete;

    void Close() {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (connectionHandle_ != nullptr) {
            for (const auto &sub : subscriptions_) {
                sub->Close();
            }

            dxf_close_connection(connectionHandle_);
            connectionHandle_ = nullptr;
        }
    }

    ~Connection() { Close(); }

    ConnectionStatus getConnectionStatus() const {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (connectionHandle_ != nullptr) {
            dxf_connection_status_t status{};
            dxf_get_current_connection_status(connectionHandle_, &status);

            return ConnectionStatus::get(status);
        }

        return ConnectionStatus::NOT_CONNECTED;
    }

    Handler<void()> &onDisconnect() { return onDisconnect_; }

    Handler<void(ConnectionStatus, ConnectionStatus)> &onConnectionStatusChanged() {
        return onConnectionStatusChanged_;
    }

    template <typename OnDisconnectListener = typename Handler<void()>::ListenerType,
              typename OnConnectionStatusChangedListener =
                  typename Handler<void(const ConnectionStatus &, const ConnectionStatus &)>::ListenerType>
    static Ptr create(const std::string &address, OnDisconnectListener &&onDisconnectListener,
                                              OnConnectionStatusChangedListener &&onConnectionStatusChangedListener) {
        return createImpl(address,
                          [&onDisconnectListener, &onConnectionStatusChangedListener](Ptr &c) {
                              c->onDisconnect() += std::forward<OnDisconnectListener>(onDisconnectListener);
                              c->onConnectionStatusChanged() +=
                                  std::forward<OnConnectionStatusChangedListener>(onConnectionStatusChangedListener);
                          });
    }

    static Ptr create(const std::string &address) {
        return createImpl(address, [](Ptr &) {});
    }

    Subscription::Ptr createSubscription(const EventTypesMask &eventTypesMask) {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (connectionHandle_ == nullptr) {
            return Subscription::INVALID;
        }

        auto sub = Subscription::create(connectionHandle_, eventTypesMask);

        if (sub) {
            subscriptions_.push_back(sub);
        }

        return sub;
    }

    template <typename EventTypeIt> Subscription::Ptr createSubscription(EventTypeIt begin, EventTypeIt end) {
        return createSubscription(EventTypesMask(begin, end));
    }

    Subscription::Ptr createSubscription(std::initializer_list<EventType> eventTypes) {
        return createSubscription(eventTypes.begin(), eventTypes.end());
    }
};

const Connection::Ptr Connection::INVALID{new Connection{}};

} // namespace dxfcpp
