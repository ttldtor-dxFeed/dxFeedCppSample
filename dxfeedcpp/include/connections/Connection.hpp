#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include <memory>

extern "C" {
#include <DXFeed.h>
}

#include <functional>
#include <mutex>
#include <string>
#include <utility>

#include "common/DXFCppConfig.hpp"

#include "helpers/Handler.hpp"

#include "ConnectionStatus.hpp"

#include "subscriptions/Subscription.hpp"

namespace dxfcpp {

/**
 * TODO:
 */
struct Connection final : public std::enable_shared_from_this<Connection> {
    ///
    using Ptr = std::shared_ptr<Connection>;

    ///
    static const Ptr INVALID;

  private:
    mutable std::recursive_mutex mutex_{};
    dxf_connection_t connectionHandle_ = nullptr;

    Handler<void()> onDisconnect_{};
    Handler<void(ConnectionStatus, ConnectionStatus)> onConnectionStatusChanged_{};
    Handler<void()> onClose_{};

    std::vector<Subscription::Ptr> subscriptions_{};
    std::vector<TimeSeriesSubscription::Ptr> timeSeriesSubscriptions_{};

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

    void close() {
        onClose_();

        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (connectionHandle_ != nullptr) {
            for (const auto &sub : subscriptions_) {
                sub->close();
            }

            for (const auto &sub : timeSeriesSubscriptions_) {
                sub->close();
            }

            dxf_close_connection(connectionHandle_);
            connectionHandle_ = nullptr;
        }
    }

  public:
    Connection &operator=(Connection &) = delete;

    ~Connection() { close(); }

    ///
    ConnectionStatus getConnectionStatus() const {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (connectionHandle_ != nullptr) {
            dxf_connection_status_t status{};
            dxf_get_current_connection_status(connectionHandle_, &status);

            return ConnectionStatus::get(status);
        }

        return ConnectionStatus::NOT_CONNECTED;
    }

    ///
    Handler<void()> &onDisconnect() { return onDisconnect_; }

    Handler<void(ConnectionStatus, ConnectionStatus)> &onConnectionStatusChanged() {
        return onConnectionStatusChanged_;
    }

    ///
    Handler<void()> &onClose() { return onClose_; }

    /**
     *
     * @tparam OnDisconnectListener
     * @tparam OnConnectionStatusChangedListener
     * @param address
     * @param onDisconnectListener
     * @param onConnectionStatusChangedListener
     * @return
     */
    template <typename OnDisconnectListener = typename Handler<void()>::ListenerType,
              typename OnConnectionStatusChangedListener =
                  typename Handler<void(const ConnectionStatus &, const ConnectionStatus &)>::ListenerType>
    static Ptr create(const std::string &address, OnDisconnectListener &&onDisconnectListener,
                      OnConnectionStatusChangedListener &&onConnectionStatusChangedListener) {
        return createImpl(address, [&onDisconnectListener, &onConnectionStatusChangedListener](Ptr &c) {
            c->onDisconnect() += std::forward<OnDisconnectListener>(onDisconnectListener);
            c->onConnectionStatusChanged() +=
                std::forward<OnConnectionStatusChangedListener>(onConnectionStatusChangedListener);
        });
    }

    /**
     *
     * @param address
     * @return
     */
    static Ptr create(const std::string &address) {
        return createImpl(address, [](Ptr &) {});
    }

    /**
     *
     * @param eventTypesMask
     * @return
     */
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

    /**
     *
     * @tparam EventTypeIt
     * @param begin
     * @param end
     * @return
     */
    template <typename EventTypeIt> Subscription::Ptr createSubscription(EventTypeIt begin, EventTypeIt end) {
        return createSubscription(EventTypesMask(begin, end));
    }

    /**
     *
     * @param eventTypes
     * @return
     */
    Subscription::Ptr createSubscription(std::initializer_list<EventType> eventTypes) {
        return createSubscription(eventTypes.begin(), eventTypes.end());
    }

    /**
     *
     * @param eventTypesMask
     * @param fromTime
     * @return
     */
    TimeSeriesSubscription::Ptr createTimeSeriesSubscription(const EventTypesMask &eventTypesMask,
                                                             std::uint64_t fromTime) {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (connectionHandle_ == nullptr) {
            return TimeSeriesSubscription::INVALID;
        }

        auto sub = TimeSeriesSubscription::create(connectionHandle_, eventTypesMask, fromTime);

        if (sub) {
            timeSeriesSubscriptions_.push_back(sub);
        }

        return sub;
    }

    /**
     *
     * @tparam EventTypeIt
     * @param begin
     * @param end
     * @param fromTime
     * @return
     */
    template <typename EventTypeIt>
    Subscription::Ptr createTimeSeriesSubscription(EventTypeIt begin, EventTypeIt end, std::uint64_t fromTime) {
        return createTimeSeriesSubscription(EventTypesMask(begin, end), fromTime);
    }

    /**
     *
     * @param eventTypes
     * @param fromTime
     * @return
     */
    Subscription::Ptr createTimeSeriesSubscription(std::initializer_list<EventType> eventTypes,
                                                   std::uint64_t fromTime) {
        return createTimeSeriesSubscription(eventTypes.begin(), eventTypes.end(), fromTime);
    }

    /**
     * Returns a Future with a vector of smart pointers to the TimeSeries instance of the object
     * (for example, dxfcpp::Candle), or a Future with an empty vector if an error occurred.
     *
     * @tparam E TimeSeries class type, e.g. dxfcpp::Candle etc
     * @param symbol
     * @param fromTime
     * @param toTime
     * @param timeout
     * @return a Future with a vector of smart pointers
     */
    template <typename E>
    std::future<std::vector<typename E::Ptr>> getTimeSeriesFuture(const std::string &symbol, std::uint64_t fromTime,
                                                                  std::uint64_t toTime, long timeout) {
        return TimeSeriesSubscriptionFuture<E>::template create<Connection>(shared_from_this(), symbol, fromTime,
                                                                            toTime, timeout);
    }

    /**
     *
     * @tparam E
     * @param symbol
     * @param fromTime
     * @param toTime
     * @param timeout
     * @return
     */
    template <typename E>
    std::future<std::vector<typename E::Ptr>>
    getTimeSeriesFuture(const std::string &symbol, std::chrono::milliseconds fromTime, std::chrono::milliseconds toTime,
                        std::chrono::seconds timeout) {
        return getTimeSeriesFuture<E>(symbol, fromTime.count(), toTime.count(), timeout.count());
    }
};

///
const Connection::Ptr Connection::INVALID{new Connection{}};

} // namespace dxfcpp
