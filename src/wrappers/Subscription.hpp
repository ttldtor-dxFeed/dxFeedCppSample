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
    class Impl {
        mutable std::recursive_mutex mutex_{};
        dxf_subscription_t subscriptionHandle_ = nullptr;
        std::function<void(int /* eventType */, dxf_const_string_t /* symbolName */,
                           const dxf_event_data_t * /* eventData */, int /*dataCount (always 1) */,
                           void * /* userData */)>
            eventListener_{};

        Handler<void(Event::Ptr)> onEvent_{};

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

        Handler<void(Event::Ptr)> &onEvent() { return onEvent_; }

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

    using Ptr = std::shared_ptr<Impl>;

    static const Ptr INVALID;

    static Ptr create(dxf_connection_t connectionHandle, const EventTypesMask &eventTypesMask) {
        auto s = std::make_shared<Impl>();
        dxf_subscription_t subscriptionHandle = nullptr;

        auto r =
            dxf_create_subscription(connectionHandle, static_cast<int>(eventTypesMask.getMask()), &subscriptionHandle);

        if (r == DXF_FAILURE) {
            return INVALID;
        }

        auto eventListener = [](int eventType, dxf_const_string_t symbolName, const dxf_event_data_t *eventData,
                                int /*dataCount (always 1) */, void *userData) {
            auto symbol = StringConverter::wStringToUtf8(symbolName);

            switch (static_cast<unsigned>(eventType)) {
            case DXF_ET_QUOTE: {
                auto cApiQuote = *reinterpret_cast<const dxf_quote_t *>(eventData);
                auto quote = std::make_shared<Quote>(symbol, cApiQuote);

                reinterpret_cast<Impl *>(userData)->onEvent_(quote);
            } break;

            case DXF_ET_CANDLE: {
                auto cApiCandle = *reinterpret_cast<const dxf_candle_t *>(eventData);
                auto candle = std::make_shared<Candle>(symbol, cApiCandle);

                reinterpret_cast<Impl *>(userData)->onEvent_(candle);
            } break;
            }

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

const Subscription::Ptr Subscription::INVALID{new Subscription::Impl{}};

} // namespace dxfcpp