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

struct SubscriptionBase {
    using Event = nonstd::variant<Quote, Candle>;
    using TimeSeriesEvent = nonstd::variant<Candle>;
    using IndexedEvent = nonstd::variant<Candle>;
};

struct Subscription : public SubscriptionBase {
    template <typename E> class Impl {
        mutable std::recursive_mutex mutex_{};
        dxf_subscription_t subscriptionHandle_ = nullptr;
        std::function<void(int /* eventType */, dxf_const_string_t /* symbolName */,
                           const dxf_event_data_t * /* eventData */, int /*dataCount (always 1) */,
                           void * /* userData */)>
            eventListener_{};

        Handler<void(const Event &)> eventsHandler_{};

        friend Subscription;

      public:
        ~Impl() {
            std::lock_guard<std::recursive_mutex> lock{mutex_};

            if (subscriptionHandle_ != nullptr) {
                dxf_close_subscription(subscriptionHandle_);
                subscriptionHandle_ = nullptr;
            }
        }
    };

    static std::shared_ptr<Impl<Event>> create(dxf_connection_t connectionHandle,
                                               const EventTypesMask &eventTypesMask) {
        auto s = std::make_shared<Impl<Event>>();
        dxf_subscription_t subscriptionHandle = nullptr;

        auto r =
            dxf_create_subscription(connectionHandle, static_cast<int>(eventTypesMask.getMask()), &subscriptionHandle);

        if (r == DXF_FAILURE) {
            return {};
        }

        auto eventListener = [](int eventType, dxf_const_string_t symbolName, const dxf_event_data_t *eventData,
                                int /*dataCount (always 1) */, void *userData) {
            auto symbol = StringConverter::wStringToUtf8(symbolName);

            switch (static_cast<unsigned>(eventType)) {
            case DXF_ET_QUOTE:
                reinterpret_cast<Impl<Event> *>(userData)->eventsHandler_(
                    Quote(symbol, *reinterpret_cast<const dxf_quote_t *>(eventData)));
            case DXF_ET_CANDLE:
                reinterpret_cast<Impl<Event> *>(userData)->eventsHandler_(
                    Candle(symbol, *reinterpret_cast<const dxf_candle_t *>(eventData)));
            }

            return;
        };

        dxf_attach_event_listener(subscriptionHandle, eventListener, reinterpret_cast<void *>(s.get()));

        s->subscriptionHandle_ = subscriptionHandle;
        s->eventListener_ = eventListener;

        return s;
    }

    void addSymbol(const std::string &symbol);

    void removeSymbol(const std::string &symbol);

    void addSymbols(const std::vector<std::string> &symbols);

    void removeSymbols(const std::vector<std::string> &symbols);

    void clearSymbols();
};

} // namespace dxfcpp