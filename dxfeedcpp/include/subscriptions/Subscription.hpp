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

/**
 *
 * @tparam T
 * @tparam Compare
 * @param v
 * @param lo
 * @param hi
 * @param comp
 * @return
 */
template <class T, class Compare> constexpr const T &clamp(const T &v, const T &lo, const T &hi, Compare comp) {
    return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

/**
 *
 * @tparam T
 * @param v
 * @param lo
 * @param hi
 * @return
 */
template <class T> constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
    return clamp(v, lo, hi, std::less<T>{});
}

///
class Symbols {
    std::vector<std::wstring> wSymbols_;
    std::vector<const wchar_t *> rawWSymbols_;

  public:
    Symbols() = delete;
    Symbols(const Symbols &) = delete;
    Symbols &operator=(const Symbols &) = delete;
    Symbols(Symbols &&) = delete;

    /**
     *
     * @tparam It
     * @param begin
     * @param end
     */
    template <typename It> Symbols(It begin, It end) {
        std::transform(begin, end, std::back_inserter(wSymbols_),
                       [](const std::string &s) { return StringConverter::utf8ToWString(s); });
        std::transform(wSymbols_.begin(), wSymbols_.end(), std::back_inserter(rawWSymbols_),
                       [](const std::wstring &s) { return s.c_str(); });
    }

    ///
    const std::vector<std::wstring> &getWSymbols() const { return wSymbols_; }

    ///
    const std::vector<const wchar_t *> &getRawWSymbols() const { return rawWSymbols_; }
};

struct Subscription;
struct TimeSeriesSubscription;

// Thread safe
class SubscriptionImpl {
    mutable std::recursive_mutex mutex_{};
    dxf_subscription_t subscriptionHandle_ = nullptr;
    std::function<void(int /* eventType */, dxf_const_string_t /* symbolName */,
                       const dxf_event_data_t * /* eventData */, int /*dataCount (always 1) */, void * /* userData */)>
        eventListener_{};

    Handler<void(Event::Ptr)> onEvent_{};

    friend Subscription;
    friend TimeSeriesSubscription;

    template <typename F> void safeCall(F &&f) {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (subscriptionHandle_ != nullptr) {
            std::forward<F>(f)(subscriptionHandle_);
        }
    }

  public:
    ///
    using Ptr = std::shared_ptr<SubscriptionImpl>;
    ///
    static const Ptr INVALID;

    ///
    void close() {
        std::lock_guard<std::recursive_mutex> lock{mutex_};

        if (subscriptionHandle_ != nullptr) {
            dxf_close_subscription(subscriptionHandle_);
            subscriptionHandle_ = nullptr;
        }
    }

    ///
    ~SubscriptionImpl() { close(); }

    ///
    Handler<void(Event::Ptr)> &onEvent() { return onEvent_; }

    /**
     *
     * @param symbol
     */
    void addSymbol(const std::string &symbol) {
        safeCall([&symbol](dxf_subscription_t sub) {
            auto wSymbol = StringConverter::utf8ToWString(symbol);

            dxf_add_symbol(sub, wSymbol.c_str());
        });
    }

    /**
     *
     * @tparam SymbolsIt
     * @param begin
     * @param end
     */
    template <typename SymbolsIt> void addSymbols(SymbolsIt begin, SymbolsIt end) {
        safeCall([&begin, &end](dxf_subscription_t sub) {
            Symbols s(begin, end);

            int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
            dxf_add_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
        });
    }

    /**
     *
     * @param symbols
     */
    void addSymbols(std::initializer_list<std::string> symbols) { return addSymbols(symbols.begin(), symbols.end()); }

    template <typename Cont> void addSymbols(Cont &&cont) {
        return addSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
    }

    /**
     *
     * @param symbol
     */
    void removeSymbol(const std::string &symbol) {
        safeCall([&symbol](dxf_subscription_t sub) {
            auto wSymbol = StringConverter::utf8ToWString(symbol);

            dxf_remove_symbol(sub, wSymbol.c_str());
        });
    }

    /**
     *
     * @tparam SymbolsIt
     * @param begin
     * @param end
     */
    template <typename SymbolsIt> void removeSymbols(SymbolsIt begin, SymbolsIt end) {
        safeCall([&begin, &end](dxf_subscription_t sub) {
            Symbols s(begin, end);

            int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
            dxf_remove_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
        });
    }

    /**
     *
     * @param symbols
     */
    void removeSymbols(std::initializer_list<std::string> symbols) {
        return removeSymbols(symbols.begin(), symbols.end());
    }

    /**
     *
     * @tparam Cont
     * @param cont
     */
    template <typename Cont> void removeSymbols(Cont &&cont) {
        return removeSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
    }

    /**
     *
     * @tparam SymbolsIt
     * @param begin
     * @param end
     */
    template <typename SymbolsIt> void setSymbols(SymbolsIt begin, SymbolsIt end) {
        safeCall([&begin, &end](dxf_subscription_t sub) {
            Symbols s(begin, end);

            int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
            dxf_set_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
        });
    }

    /**
     *
     * @param symbols
     */
    void setSymbols(std::initializer_list<std::string> symbols) { return setSymbols(symbols.begin(), symbols.end()); }

    /**
     *
     * @tparam Cont
     * @param cont
     */
    template <typename Cont> void setSymbols(Cont &&cont) {
        return setSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
    }

    ///
    void clearSymbols() {
        safeCall([](dxf_subscription_t sub) { dxf_clear_symbols(sub); });
    }

    ///
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
            }

            // TODO: add events
            return;
        };
    }
};

///
const SubscriptionImpl::Ptr SubscriptionImpl::INVALID{new SubscriptionImpl{}};

/**
 *
 */
struct Subscription final {
    using Ptr = std::shared_ptr<SubscriptionImpl>;

    static const Ptr INVALID;

    /**
     *
     * @param connectionHandle
     * @param eventTypesMask
     * @return
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

///
const Subscription::Ptr Subscription::INVALID{SubscriptionImpl::INVALID};

///
struct TimeSeriesSubscription {
    ///
    using Ptr = std::shared_ptr<SubscriptionImpl>;

    ///
    static const Ptr INVALID;

    /**
     *
     * @param connectionHandle
     * @param eventTypesMask
     * @param fromTime
     * @return
     */
    static Ptr create(dxf_connection_t connectionHandle, const EventTypesMask &eventTypesMask, std::uint64_t fromTime) {
        auto s = std::make_shared<SubscriptionImpl>();
        dxf_subscription_t subscriptionHandle = nullptr;

        auto r = dxf_create_subscription_timed(connectionHandle, static_cast<int>(eventTypesMask.getMask()),
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

///
const TimeSeriesSubscription::Ptr TimeSeriesSubscription::INVALID{SubscriptionImpl::INVALID};

struct Connection;

/**
 *
 * @tparam E
 */
template <typename E> struct TimeSeriesSubscriptionFuture {
    ///
    class HistoryBuffer {
        std::atomic<bool> done_{false};

        std::mutex eventsMutex_{};
        std::map<std::uint64_t, typename E::Ptr> events_{};

        std::mutex cvMutex_{};
        std::condition_variable cv_{};

        std::uint64_t fromTime_;
        std::uint64_t toTime_;

      public:
        /**
         *
         * @param fromTime
         * @param toTime
         */
        HistoryBuffer(std::uint64_t fromTime, std::uint64_t toTime)
            : fromTime_{fromTime}, toTime_{toTime} {}

        /**
         *
         * @param timeout
         */
        void wait(long long timeout) {
            std::unique_lock<std::mutex> lk(cvMutex_);
            if (timeout == 0) {
                cv_.wait(lk, [this] { return done_.load(); });
            } else {
                cv_.wait_for(lk, std::chrono::seconds(timeout), [this] { return done_.load(); });
            }
        }

        ///
        void done() {
            done_ = true;
            cv_.notify_one();
        }

        /**
         *
         * @param e
         */
        void applyEventData(Event::Ptr e) {
            // Do double-check
            if (!e)
                return;

            if (!e->is<E>())
                return;

            auto event = e->sharedAs<E>();

            if (!event)
                return;

            typename E::Ptr copy = std::make_shared<E>(*event);

            std::lock_guard<std::mutex> guard(eventsMutex_);

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
                done();
            }
        }

        ///
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
     *
     * @tparam Connection
     * @param connection
     * @param symbol
     * @param fromTime
     * @param toTime
     * @param timeout
     * @return
     */
    template <typename Connection>
    static std::future<std::vector<typename E::Ptr>> create(typename Connection::Ptr connection,
                                                            const std::string &symbol, std::uint64_t fromTime,
                                                            std::uint64_t toTime, long timeout) {
        return std::async(
            std::launch::async,
            [](typename Connection::Ptr connection, const std::string &symbol, std::uint64_t fromTime,
               std::uint64_t toTime, long timeout) -> std::vector<typename E::Ptr> {
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