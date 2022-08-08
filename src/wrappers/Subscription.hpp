#pragma once

#include <DXFeed.h>
#include <EventData.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "DXFCppConfig.hpp"

#include "EventTraits.hpp"
#include "EventType.hpp"

#include <variant.hpp>

#include "helpers/Handler.hpp"

#include "Candle.hpp"
#include "Quote.hpp"

namespace dxfcpp {

struct Connection;

template <class T, class Compare> constexpr const T &clamp(const T &v, const T &lo, const T &hi, Compare comp) {
    return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}

template <class T> constexpr const T &clamp(const T &v, const T &lo, const T &hi) {
    return clamp(v, lo, hi, std::less<T>{});
}

class Symbols {
    std::vector<std::wstring> wSymbols_;
    std::vector<const wchar_t *> rawWSymbols_;

  public:
    Symbols() = delete;
    Symbols(const Symbols &) = delete;
    Symbols &operator=(const Symbols &) = delete;
    Symbols(Symbols &&) = delete;

    template <typename It> Symbols(It begin, It end) {
        std::transform(begin, end, std::back_inserter(wSymbols_),
                       [](const std::string &s) { return StringConverter::utf8ToWString(s); });
        std::transform(wSymbols_.begin(), wSymbols_.end(), std::back_inserter(rawWSymbols_),
                       [](const std::wstring &s) { return s.c_str(); });
    }

    const std::vector<std::wstring> &getWSymbols() const { return wSymbols_; }

    const std::vector<const wchar_t *> &getRawWSymbols() const { return rawWSymbols_; }
};

struct Subscription final {
    using Event = nonstd::variant<Quote, Candle>;
    using TimeSeriesEvent = nonstd::variant<Candle>;
    using IndexedEvent = nonstd::variant<Candle>;

    template <typename E> class Impl {
        mutable std::recursive_mutex mutex_{};
        dxf_subscription_t subscriptionHandle_ = nullptr;
        std::function<void(int /* eventType */, dxf_const_string_t /* symbolName */,
                           const dxf_event_data_t * /* eventData */, int /*dataCount (always 1) */,
                           void * /* userData */)>
            eventListener_{};

        Handler<void(const Event &)> onEvent_{};

        friend Subscription;

        template <typename F> void safeCall(F &&f) {
            std::lock_guard<std::recursive_mutex> lock{mutex_};

            if (subscriptionHandle_ != nullptr) {
                std::forward<F>(f)(subscriptionHandle_);
            }
        }

      public:
        void Close() {
            std::lock_guard<std::recursive_mutex> lock{mutex_};

            if (subscriptionHandle_ != nullptr) {
                dxf_close_subscription(subscriptionHandle_);
                subscriptionHandle_ = nullptr;
            }
        }

        ~Impl() { Close(); }

        Handler<void(const Event &)> &onEvent() { return onEvent_; }

        void addSymbol(const std::string &symbol) {
            safeCall([&symbol](dxf_subscription_t sub) {
                auto wSymbol = StringConverter::utf8ToWString(symbol);

                dxf_add_symbol(sub, wSymbol.c_str());
            });
        }

        template <typename SymbolsIt> void addSymbols(SymbolsIt begin, SymbolsIt end) {
            safeCall([&begin, &end](dxf_subscription_t sub) {
                Symbols s(begin, end);

                int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
                dxf_add_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
            });
        }

        void addSymbols(std::initializer_list<std::string> symbols) {
            return addSymbols(symbols.begin(), symbols.end());
        }

        template <typename Cont> void addSymbols(Cont &&cont) {
            return addSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
        }

        void removeSymbol(const std::string &symbol) {
            safeCall([&symbol](dxf_subscription_t sub) {
                auto wSymbol = StringConverter::utf8ToWString(symbol);

                dxf_remove_symbol(sub, wSymbol.c_str());
            });
        }

        template <typename SymbolsIt> void removeSymbols(SymbolsIt begin, SymbolsIt end) {
            safeCall([&begin, &end](dxf_subscription_t sub) {
                Symbols s(begin, end);

                int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
                dxf_remove_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
            });
        }

        void removeSymbols(std::initializer_list<std::string> symbols) {
            return removeSymbols(symbols.begin(), symbols.end());
        }

        template <typename Cont> void removeSymbols(Cont &&cont) {
            return removeSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
        }

        template <typename SymbolsIt> void setSymbols(SymbolsIt begin, SymbolsIt end) {
            safeCall([&begin, &end](dxf_subscription_t sub) {
                Symbols s(begin, end);

                int size = clamp(static_cast<int>(s.getRawWSymbols().size()), 0, std::numeric_limits<int>::max());
                dxf_set_symbols(sub, const_cast<dxf_const_string_t *>(s.getRawWSymbols().data()), size);
            });
        }

        void setSymbols(std::initializer_list<std::string> symbols) {
            return setSymbols(symbols.begin(), symbols.end());
        }

        template <typename Cont> void setSymbols(Cont &&cont) {
            return setSymbols(std::begin(std::forward<Cont>(cont)), std::end(std::forward<Cont>(cont)));
        }

        void clearSymbols() {
            safeCall([](dxf_subscription_t sub) { dxf_clear_symbols(sub); });
        }
    };

    using Ptr = std::shared_ptr<Impl<Event>>;

    static const Ptr INVALID;

    static Ptr create(dxf_connection_t connectionHandle, const EventTypesMask &eventTypesMask) {
        auto s = std::make_shared<Impl<Event>>();
        dxf_subscription_t subscriptionHandle = nullptr;

        auto r =
            dxf_create_subscription(connectionHandle, static_cast<int>(eventTypesMask.getMask()), &subscriptionHandle);

        if (r == DXF_FAILURE) {
            return INVALID;
        }

        auto eventListener = [](int eventType, dxf_const_string_t symbolName, const dxf_event_data_t *eventData,
                                int /*dataCount (always 1) */, void *userData) {
            auto symbol = StringConverter::wStringToUtf8(symbolName);

#define DXFCPP_SUB_EVENT_LISTENER_CASE(CAPI_EVENT_MASK, CPPAPI_EVENT_TYPE, CAPI_EVENT_TYPE)                            \
    case CAPI_EVENT_MASK:                                                                                              \
        reinterpret_cast<Impl<Event> *>(userData)->onEvent_(                                                           \
            CPPAPI_EVENT_TYPE(symbol, *reinterpret_cast<const CAPI_EVENT_TYPE *>(eventData))); break

            switch (static_cast<unsigned>(eventType)) {
                DXFCPP_SUB_EVENT_LISTENER_CASE(DXF_ET_QUOTE, Quote, dxf_quote_t);
                DXFCPP_SUB_EVENT_LISTENER_CASE(DXF_ET_CANDLE, Candle, dxf_candle_t);
            }

#undef DXFCPP_SUB_EVENT_LISTENER_CASE
            return;
        };

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

const Subscription::Ptr Subscription::INVALID{new Subscription::Impl<Event>{}};

} // namespace dxfcpp