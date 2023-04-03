#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

extern "C" {
#include <DXFeed.h>
#include <EventData.h>
}

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "common/DXFCppConfig.hpp"

#include "events/Candle.hpp"
#include "events/EventTraits.hpp"
#include "events/EventType.hpp"
#include "events/Quote.hpp"
#include "events/Summary.hpp"
#include "events/Trade.hpp"

#include "helpers/Handler.hpp"

namespace dxfcpp {

template <class T, class Compare> constexpr const T &clamp(const T &v, const T &lo, const T &hi, Compare comp) {
    return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

template <class T> constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
    return clamp(v, lo, hi, std::less<T>{});
}

/// Symbols buffer used to convert vector std::string symbols to wchar_t** symbols
class Symbols {
    std::vector<std::wstring> wSymbols_;
    std::vector<const wchar_t *> rawWSymbols_;

  public:
    Symbols() = delete;
    Symbols(const Symbols &) = delete;
    Symbols &operator=(const Symbols &) = delete;
    Symbols(Symbols &&) = delete;

    /**
     * Create the new buffer from iterators to container of strings
     *
     * @tparam It The iterator type
     * @param begin The first iterator
     * @param end The last iterator
     */
    template <typename It> Symbols(It begin, It end) {
        std::transform(begin, end, std::back_inserter(wSymbols_),
                       [](const std::string &s) { return StringConverter::utf8ToWString(s); });
        std::transform(wSymbols_.begin(), wSymbols_.end(), std::back_inserter(rawWSymbols_),
                       [](const std::wstring &s) { return s.c_str(); });
    }

    /// Returns the vector of wstring symbols
    const std::vector<std::wstring> &getWSymbols() const { return wSymbols_; }

    /// Returns the vector of const wchar_t* symbols
    const std::vector<const wchar_t *> &getRawWSymbols() const { return rawWSymbols_; }
};

struct Subscription;
struct TimeSeriesSubscription;

// A thread-safe wrapper class to hold dxf_subscription_t handle and manipulate symbols
class SubscriptionImpl {
    mutable std::recursive_mutex mutex_{};
    dxf_subscription_t subscriptionHandle_ = nullptr;
    std::function<void(int /* eventType */, dxf_const_string_t /* symbolName */,
                       const dxf_event_data_t * /* eventData */, int /*dataCount (always 1) */, void * /* userData */)>
        eventListener_{};

    Handler<void(Event::Ptr)> onEvent_{1};

    friend Subscription;
    friend TimeSeriesSubscription;

    template <typename F> void safeCall(F &&f) {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (subscriptionHandle_ != nullptr) {
            std::forward<F>(f)(subscriptionHandle_);
        }
    }

  public:
    /// The synonym for a shared pointer to a SubscriptionImpl object
    using Ptr = std::shared_ptr<SubscriptionImpl>;
    // The synonym for a weak pointer to a SubscriptionImpl object
    using WeakPtr = std::weak_ptr<SubscriptionImpl>;

    /// An invalid pointer that is returned if something went wrong. Usually, operations with an invalid pointer
    /// do not give any result, since when trying to perform an operation, the handle is checked.
    static const Ptr INVALID;

    /// Tries to close the current subscription
    void close() {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (subscriptionHandle_ != nullptr) {
            dxf_close_subscription(subscriptionHandle_);
            subscriptionHandle_ = nullptr;
        }
    }

    /// RAII
    ~SubscriptionImpl() { close(); }

    /// Returns the onEvent handler that notifies all listeners asynchronously that the new event has been received
    Handler<void(Event::Ptr)> &onEvent() { return onEvent_; }

    /**
     * Adds the symbol to subscription
     *
     * @param symbol The symbol to subscribe
     */
    void addSymbol(const std::string &symbol) {
        safeCall([&symbol](dxf_subscription_t sub) {
            auto wSymbol = StringConverter::utf8ToWString(symbol);

            dxf_add_symbol(sub, wSymbol.c_str());
        });
    }

    /**
     * Adds the symbols to subscription
     *
     * @tparam SymbolsIt The type of iterator of the symbols container
     * @param begin The first iterator of symbols container
     * @param end The last iterator of symbols container
     */
    template <typename SymbolsIt> void addSymbols(SymbolsIt begin, SymbolsIt end) {
        safeCall([&begin, &end](dxf_subscription_t sub) {
            Symbols s(begin, end);

            int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
            dxf_add_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
        });
    }

    /**
     * Adds the symbols to subscription
     *
     * @param symbols The initializer list of symbols
     */
    void addSymbols(std::initializer_list<std::string> symbols) { return addSymbols(symbols.begin(), symbols.end()); }

    /**
     * Adds the symbols to subscription
     *
     * @tparam Cont The type of container of symbols
     * @param cont The container of symbols
     */
    template <typename Cont> void addSymbols(Cont &&cont) {
        return addSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
    }

    /**
     * Removes the symbol from subscription
     *
     * @param symbol The symbol to remove
     */
    void removeSymbol(const std::string &symbol) {
        safeCall([&symbol](dxf_subscription_t sub) {
            auto wSymbol = StringConverter::utf8ToWString(symbol);

            dxf_remove_symbol(sub, wSymbol.c_str());
        });
    }

    /**
     * Removes the symbols from subscription
     *
     * @tparam SymbolsIt The type of iterator of the symbols container
     * @param begin The first iterator of symbols container
     * @param end The last iterator of symbols container
     */
    template <typename SymbolsIt> void removeSymbols(SymbolsIt begin, SymbolsIt end) {
        safeCall([&begin, &end](dxf_subscription_t sub) {
            Symbols s(begin, end);

            int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
            dxf_remove_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
        });
    }

    /**
     * Removes the symbols from subscription
     *
     * @param symbols The initializer list of symbols
     */
    void removeSymbols(std::initializer_list<std::string> symbols) {
        return removeSymbols(symbols.begin(), symbols.end());
    }

    /**
     * Removes the symbols from subscription
     *
     * @tparam Cont The type of container of symbols
     * @param cont The container of symbols
     */
    template <typename Cont> void removeSymbols(Cont &&cont) {
        return removeSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
    }

    /**
     * Sets the symbols for the subscription. Clears symbols and adds.
     *
     * @tparam SymbolsIt The type of iterator of the symbols container
     * @param begin The first iterator of symbols container
     * @param end The last iterator of symbols container
     */
    template <typename SymbolsIt> void setSymbols(SymbolsIt begin, SymbolsIt end) {
        safeCall([&begin, &end](dxf_subscription_t sub) {
            Symbols s(begin, end);

            int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
            dxf_set_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
        });
    }

    /**
     * Sets the symbols for the subscription. Clears symbols and adds.
     *
     * @param symbols The initializer list of symbols
     */
    void setSymbols(std::initializer_list<std::string> symbols) { return setSymbols(symbols.begin(), symbols.end()); }

    /**
     * Sets the symbols for the subscription. Clears symbols and adds.
     *
     * @tparam Cont The type of container of symbols
     * @param cont The container of symbols
     */
    template <typename Cont> void setSymbols(Cont &&cont) {
        return setSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
    }

    /// Clears the subscription's symbols
    void clearSymbols() {
        safeCall([](dxf_subscription_t sub) { dxf_clear_symbols(sub); });
    }

    /// Creates the generic events listener that used by subscription wrappers.
    static dxf_event_listener_t createEventListener() {
        return [](int eventType, dxf_const_string_t symbolName, const dxf_event_data_t *eventData,
                  int /*dataCount (always 1) */, void *userData) {
            auto symbol = StringConverter::wStringToUtf8(symbolName);

            switch (static_cast<unsigned>(eventType)) {
            case DXF_ET_QUOTE: {
                auto cApiQuote = *reinterpret_cast<const dxf_quote_t *>(eventData);
                auto quote = std::make_shared<Quote>(symbol, cApiQuote);

                reinterpret_cast<SubscriptionImpl *>(userData)->onEvent_(quote);
            } break;

            case DXF_ET_CANDLE: {
                auto cApiCandle = *reinterpret_cast<const dxf_candle_t *>(eventData);
                auto candle = std::make_shared<Candle>(symbol, cApiCandle);

                reinterpret_cast<SubscriptionImpl *>(userData)->onEvent_(candle);
            } break;

            case DXF_ET_TRADE: {
                auto cApiTrade = *reinterpret_cast<const dxf_trade_t *>(eventData);
                auto trade = std::make_shared<Trade>(symbol, cApiTrade);

                reinterpret_cast<SubscriptionImpl *>(userData)->onEvent_(trade);
            } break;

            case DXF_ET_TRADE_ETH: {
                auto cApiTradeEth = *reinterpret_cast<const dxf_trade_eth_t *>(eventData);
                auto tradeEth = std::make_shared<TradeETH>(symbol, cApiTradeEth);

                reinterpret_cast<SubscriptionImpl *>(userData)->onEvent_(tradeEth);
            } break;

            case DXF_ET_SUMMARY: {
                auto cApiSummary = *reinterpret_cast<const dxf_summary_t *>(eventData);
                auto summary = std::make_shared<Summary>(symbol, cApiSummary);

                reinterpret_cast<SubscriptionImpl *>(userData)->onEvent_(summary);
            } break;
            }

            // TODO: add events
            return;
        };
    }
};

const SubscriptionImpl::Ptr SubscriptionImpl::INVALID{new SubscriptionImpl{}};

/**
 * The thread-safe wrapper class to create subscriptions to events
 */
struct Subscription final {
    /// The synonym for a shared pointer to a Subscription object
    using Ptr = std::shared_ptr<SubscriptionImpl>;
    // The synonym for a weak pointer to a Subscription object
    using WeakPtr = std::weak_ptr<SubscriptionImpl>;

    /// An invalid pointer that is returned if something went wrong. Usually, operations with an invalid pointer
    /// do not give any result, since when trying to perform an operation, the handle is checked.
    static const Ptr INVALID;

    /**
     * Creates the new subscription to events.
     *
     * @param connectionHandle The parent connection handle
     * @param eventTypesMask The flags mask of events to subscribe
     * @return A shared pointer to the new Subscription object or Subscription::INVALID
     */
    static Ptr create(dxf_connection_t connectionHandle, const EventTypesMask &eventTypesMask) {
        auto s = std::make_shared<SubscriptionImpl>();
        dxf_subscription_t subscriptionHandle = nullptr;

        auto r =
            dxf_create_subscription(connectionHandle, static_cast<int>(eventTypesMask.getMask()), &subscriptionHandle);

        if (r == DXF_FAILURE) {
            return INVALID;
        }

        auto eventListener = SubscriptionImpl::createEventListener();

        r = dxf_attach_event_listener(subscriptionHandle, eventListener, reinterpret_cast<void *>(s.get()));

        if (r == DXF_FAILURE) {
            dxf_close_subscription(subscriptionHandle);

            return INVALID;
        }

        s->subscriptionHandle_ = subscriptionHandle;
        s->eventListener_ = eventListener;

        return s;
    }
};

const Subscription::Ptr Subscription::INVALID{SubscriptionImpl::INVALID};

/// The thread-safe wrapper class to create subscriptions to TimeSeries events
struct TimeSeriesSubscription {
    /// The synonym for a shared pointer to a TimeSeriesSubscription object
    using Ptr = std::shared_ptr<SubscriptionImpl>;
    /// The synonym for a weak pointer to a TimeSeriesSubscription object
    using WeakPtr = std::weak_ptr<SubscriptionImpl>;

    /// An invalid pointer that is returned if something went wrong. Usually, operations with an invalid pointer
    /// do not give any result, since when trying to perform an operation, the handle is checked.
    static const Ptr INVALID;

    /**
     * Creates the new subscription to time series events with specified fromTime. Also removes non time series events
     * from the events mask
     *
     * @param connectionHandle The connection handle
     * @param eventTypesMask The event types mask to subscribe
     * @param fromTime The time from which data must be requested
     * @return A shared pointer to the new TimeSeriesSubscription object or TimeSeriesSubscription::INVALID
     */
    static Ptr create(dxf_connection_t connectionHandle, const EventTypesMask &eventTypesMask, std::uint64_t fromTime) {
        auto s = std::make_shared<SubscriptionImpl>();
        dxf_subscription_t subscriptionHandle = nullptr;

        auto onlyTimeSeries = eventTypesMask & EventTypesMask::TIME_SERIES;

        auto r = dxf_create_subscription_timed(connectionHandle, static_cast<int>(onlyTimeSeries.getMask()),
                                               static_cast<dxf_long_t>(fromTime), &subscriptionHandle);

        if (r == DXF_FAILURE) {
            return INVALID;
        }

        auto eventListener = SubscriptionImpl::createEventListener();

        r = dxf_attach_event_listener(subscriptionHandle, eventListener, reinterpret_cast<void *>(s.get()));

        if (r == DXF_FAILURE) {
            dxf_close_subscription(subscriptionHandle);

            return INVALID;
        }

        s->subscriptionHandle_ = subscriptionHandle;
        s->eventListener_ = eventListener;

        return s;
    }
};

const TimeSeriesSubscription::Ptr TimeSeriesSubscription::INVALID{SubscriptionImpl::INVALID};

struct Connection;

/**
 * A thread-safe wrapper over a TimeSeriesSubscription that subscribes to one type, one symbol from time fromTime to
 * time toTime. Collects incoming events into a buffer, given time, indexes, and flags. Returns the future with a vector
 * of events ordered by time back to the past (from toTime to fromTime)
 *
 * @tparam E The type of the event to subscribe
 */
template <typename E> struct TimeSeriesSubscriptionFuture {
    /// An internal thread-safe history buffer that collects incoming events, taking into account time, indexes and
    /// flags. Returns a vector of events sorted by time back into the past (from toTime to fromTime). Can wait for
    /// events or a shutdown signal.
    class HistoryBuffer {
        std::atomic<bool> done_{false};

        std::mutex eventsMutex_{};
        std::map<std::uint64_t, typename E::Ptr> events_{};
        std::condition_variable cv_{};

        std::uint64_t fromTime_;
        std::uint64_t toTime_;

      public:
        /**
         * Creates a buffer with the specified filtering parameters (from what time and until what time).
         *
         * @param fromTime The time from which to collect events.
         * @param toTime The time after which events should be ignored and work should be completed.
         */
        HistoryBuffer(std::uint64_t fromTime, std::uint64_t toTime) : fromTime_{fromTime}, toTime_{toTime} {}

        /**
         * Waits for an internal signal that all data has been received, or for an external event that the work needs to
         * be completed.
         *
         * @param timeout Waits for an internal signal that the data has been received, or for an external event that
         * the work needs to be completed.
         */
        void wait(long long timeout) {
            std::unique_lock<std::mutex> lk(eventsMutex_);
            if (timeout == 0) {
                cv_.wait(lk, [this] { return done_.load(); });
            } else {
                cv_.wait_for(lk, std::chrono::seconds(timeout), [this] { return done_.load(); });
            }
        }

        /// Allows you to tell the buffer that work should be completed.
        void done() {
            done_ = true;
            cv_.notify_one();
        }

        /**
         * "Applies" the event to the buffer.
         * Takes into account flags, time, index and adds an event to the buffer, if necessary, or removes the event
         * from the buffer.
         * @param e A pointer to the event
         */
        void applyEventData(Event::Ptr e) {
            auto event = e->sharedAs<E>();

            if (!event)
                return;

            typename E::Ptr copy = std::make_shared<E>(*event);

            std::unique_lock<std::mutex> lk(eventsMutex_);

            if (copy->getTime() >= fromTime_ && copy->getTime() <= toTime_) {
                bool remove = dxfcpp::EventFlag::REMOVE_EVENT.in(copy->getEventFlags());

                // Clear the event flags
                copy->setEventFlags(dxfcpp::EventFlagsMask());

                auto found = events_.find(copy->getIndex());

                if (found == events_.end()) {
                    if (!remove) {
                        events_.emplace(copy->getIndex(), copy);
                    }
                } else if (remove) {
                    events_.erase(found);
                } else {
                    found->second = copy;
                }
            }

            if (copy->getTime() <= fromTime_ || EventFlag::SNAPSHOT_SNIP.in(copy->getEventFlags())) {
                lk.unlock();
                done();
                lk.lock();
            }
        }

        /// Returns the result sorted by time in reverse order.
        std::vector<typename E::Ptr> getResult() {
            std::lock_guard<std::mutex> guard(eventsMutex_);

            std::vector<typename E::Ptr> result{};

            // reverse
            std::transform(events_.rbegin(), events_.rend(), std::back_inserter(result),
                           [](const std::pair<std::uint64_t, typename E::Ptr> &pair) { return pair.second; });

            return result;
        }
    };

    /**
     * Returns a future with an vector of TimeSeries of events that will be received as a result of subscribing for the
     * specified type, symbol, and time from and to
     *
     * @tparam Connection The type of parent connection
     * @param connection The parent connection
     * @param symbol The symbol to subscribe
     * @param fromTime The time from which events are buffered.
     * @param toTime The time at which events are no longer added to the buffer and work is completed.
     * @param timeout The timeout after which the work completes.
     * @return The future to the vector of time series events of empty vector
     */
    template <typename Connection>
    static std::future<std::vector<typename E::Ptr>> create(typename Connection::Ptr connection,
                                                            const std::string &symbol, std::uint64_t fromTime,
                                                            std::uint64_t toTime, long timeout) {
        return std::async(
            std::launch::async,
            [](typename Connection::Ptr connection, const std::string &symbol, std::uint64_t fromTime,
               std::uint64_t toTime, long timeout) -> std::vector<typename E::Ptr> {
                // Checks that the event type is TimeSeries. Otherwise returns an empty vector.
                if (!EventTraits<E>::isTimeSeriesEvent) {
                    return {};
                }

                auto buffer = std::make_shared<HistoryBuffer>(fromTime, toTime);
                auto sub = connection->createTimeSeriesSubscription({EventTraits<E>::getEventType()}, fromTime);

                if (sub == TimeSeriesSubscription::INVALID)
                    return {};

                sub->onEvent() += [buffer](dxfcpp::Event::Ptr e) { buffer->applyEventData(e); };
                connection->onClose() += [buffer]() { buffer->done(); };
                sub->addSymbol(symbol);

                buffer->wait(timeout);

                return buffer->getResult();
            },
            connection, symbol, fromTime, toTime, timeout);
    }
};

} // namespace dxfcpp