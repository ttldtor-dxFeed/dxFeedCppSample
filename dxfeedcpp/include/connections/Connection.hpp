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
 * The thread-safe wrapper around the dxf_connection_t dxFeed C-API type
 *
 * Allows you to create connections, close connections, reuse connections.
 * Allows you to create different types of subscriptions and use RAII to close them if necessary.
 *
 * Allows you to get the current status of the connection, as well as receive a notification if the connection was
 * interrupted from the outside (for example, the network adapter broke or the connection was interrupted by the
 * firewall).
 *
 * At the moment, the implementation does not create shared buffers for TICKER, STREAM, HISTORY contracts.
 * New subscriptions can affect old ones, since for the server it all happens in one session.
 * In other words, there is no multiplexing and subscription caching.
 */
struct Connection final : public std::enable_shared_from_this<Connection> {
    /// The synonym for an shared pointer to a Connection object
    using Ptr = std::shared_ptr<Connection>;

    /// An invalid pointer that is returned if something went wrong. Usually, operations with an invalid pointer
    /// do not give any result, since when trying to perform an operation, the handle is checked.
    static const Ptr INVALID;

  private:
    mutable std::recursive_mutex mutex_{};
    dxf_connection_t connectionHandle_ = nullptr;

    Handler<void()> onDisconnect_{1};
    Handler<void(ConnectionStatus, ConnectionStatus)> onConnectionStatusChanged_{1};
    Handler<void()> onClose_{1};

    std::vector<Subscription::WeakPtr> subscriptions_{};
    std::vector<TimeSeriesSubscription::WeakPtr> timeSeriesSubscriptions_{};

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
                if (auto s = sub.lock()) {
                    s->close();
                }
            }

            for (const auto &sub : timeSeriesSubscriptions_) {
                if (auto s = sub.lock()) {
                    s->close();
                }
            }

            dxf_close_connection(connectionHandle_);
            connectionHandle_ = nullptr;
        }
    }

  public:
    Connection &operator=(Connection &) = delete;

    /// Tries to "send" the onClose notification (used by TimeSeriesFuture) and tries to close all subscriptions
    ~Connection() { close(); }

    /// Returns the current connection status (default value = NOT_CONNECTED)
    ConnectionStatus getConnectionStatus() const {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (connectionHandle_ != nullptr) {
            dxf_connection_status_t status{};
            dxf_get_current_connection_status(connectionHandle_, &status);

            return ConnectionStatus::get(status);
        }

        return ConnectionStatus::NOT_CONNECTED;
    }

    /// Returns the onDisconnect handler that notifies all listeners asynchronously that the connection has been
    /// disconnected.
    Handler<void()> &onDisconnect() { return onDisconnect_; }

    /// Returns the onConnectionStatusChanged handler that notifies all listeners asynchronously that the connection
    /// connection status has been changed.
    Handler<void(ConnectionStatus, ConnectionStatus)> &onConnectionStatusChanged() {
        return onConnectionStatusChanged_;
    }

    /// Returns the onClose handler that notifies all listeners asynchronously that a connection has been closed.
    Handler<void()> &onClose() { return onClose_; }

    /**
     * Creates the new connection to specified address with specified onDisconnect & onConnectionStatusChanged listeners
     *
     * @tparam OnDisconnectListener The type of a onDisconnect listener (it could be any callable)
     * @tparam OnConnectionStatusChangedListener The type of onConnectionStatusChanged listener
     * @param address The address to connect
     * @param onDisconnectListener The onDisconnect listener
     * @param onConnectionStatusChangedListener The onConnectionStatusChanged listener
     * @return A shared pointer to the new connection object or Connection::INVALID
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
     * Creates the new connection to specified address
     *
     * @param address The address to connect
     * @return A shared pointer to the new connection object or Connection::INVALID
     */
    static Ptr create(const std::string &address) {
        return createImpl(address, [](Ptr &) {});
    }

    /**
     * Creates the new subscription by specified event types mask. Subscription contracts (TICKER, STREAM, HISTORY) are
     * inferred by event type in the mask.
     *
     * @param eventTypesMask The event types mask
     * @return A shared pointer to the new Subscription object or Subscription::INVALID
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
     * Creates the new subscription by specified event types which accessible from container that represented by
     * iterators. Subscription contracts (TICKER, STREAM, HISTORY) are inferred by event type.
     *
     * @tparam EventTypeIt The iterator type of the container with event types
     * @param begin The first iterator of the container with event type
     * @param end The last iterator of the container with event type
     * @return A shared pointer to the new Subscription object or Subscription::INVALID
     */
    template <typename EventTypeIt> Subscription::Ptr createSubscription(EventTypeIt begin, EventTypeIt end) {
        return createSubscription(EventTypesMask(begin, end));
    }

    /**
     * Creates the new subscription by specified event types. Subscription contracts (TICKER, STREAM, HISTORY) are
     * inferred by event type.
     *
     * @param eventTypes The initializer list with event types
     * @return A shared pointer to the new Subscription object or Subscription::INVALID
     */
    Subscription::Ptr createSubscription(std::initializer_list<EventType> eventTypes) {
        return createSubscription(eventTypes.begin(), eventTypes.end());
    }

    /**
     * Creates the new subscription to time series events with specified fromTime. Also removes non time series events
     * from the events mask
     *
     * @param eventTypesMask The event types mask to subscribe
     * @param fromTime The time from which data must be requested
     * @return A shared pointer to the new TimeSeriesSubscription object or TimeSeriesSubscription::INVALID
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
     * Creates the new subscription to time series events with specified fromTime. Also removes non time series event
     * types from the event types
     *
     * @tparam EventTypeIt The iterator type of the container with event types
     * @param begin The first iterator of the container with event type
     * @param end The last iterator of the container with event type
     * @param fromTime The time from which data must be requested
     * @return A shared pointer to the new TimeSeriesSubscription object or TimeSeriesSubscription::INVALID
     */
    template <typename EventTypeIt>
    TimeSeriesSubscription::Ptr createTimeSeriesSubscription(EventTypeIt begin, EventTypeIt end,
                                                             std::uint64_t fromTime) {
        return createTimeSeriesSubscription(EventTypesMask(begin, end), fromTime);
    }

    /**
     * Creates the new subscription to time series events with specified fromTime. Also removes non time series event
     * types from the event types
     *
     * @param eventTypes The initializer list with event types
     * @param fromTime The time from which data must be requested
     * @return A shared pointer to the new TimeSeriesSubscription object or TimeSeriesSubscription::INVALID
     */
    TimeSeriesSubscription::Ptr createTimeSeriesSubscription(std::initializer_list<EventType> eventTypes,
                                                             std::uint64_t fromTime) {
        return createTimeSeriesSubscription(eventTypes.begin(), eventTypes.end(), fromTime);
    }

    /**
     * Returns a Future with a vector of smart pointers to the TimeSeries instance of the object
     * (for example, dxfcpp::Candle), or a Future with an empty vector if an error occurred.
     *
     * If the timeout occurs before the last time series event has been received, then a future will be returned for an
     * incomplete snapshot of the time series events.
     *
     * @tparam E TimeSeries class type, e.g. dxfcpp::Candle etc
     * @param symbol The symbol to subscribe
     * @param fromTime Time from which events will be added to the snapshot (historical event buffer)
     * @param toTime The time until which events will be added to the snapshot (historical event buffer)
     * @param timeout The timeout after which the work completes.
     * @return A Future with a vector of smart pointers to TimeSeries events
     */
    template <typename E>
    std::future<std::vector<typename E::Ptr>> getTimeSeriesFuture(const std::string &symbol, std::uint64_t fromTime,
                                                                  std::uint64_t toTime, long timeout) {
        return TimeSeriesSubscriptionFuture<E>::template create<Connection>(shared_from_this(), symbol, fromTime,
                                                                            toTime, timeout);
    }

    /**
     * Returns a Future with a vector of smart pointers to the TimeSeries instance of the object
     * (for example, dxfcpp::Candle), or a Future with an empty vector if an error occurred.
     *
     * If the timeout occurs before the last time series event has been received, then a future will be returned for an
     * incomplete snapshot of the time series events.
     *
     * @tparam E TimeSeries class type, e.g. dxfcpp::Candle etc
     * @param symbol The symbol to subscribe
     * @param fromTime Time from which events will be added to the snapshot (historical event buffer)
     * @param toTime The time until which events will be added to the snapshot (historical event buffer)
     * @param timeout The timeout after which the work completes.
     * @return A Future with a vector of smart pointers to TimeSeries events
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
