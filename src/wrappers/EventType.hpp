#pragma once

#include "DXFCppConfig.hpp"

#include <unordered_set>
#include <utility>

namespace dxfcpp {

class EventType {
    unsigned cApiEventId_;
    unsigned cApiEventMask_;

    EventType(unsigned cApiEventId, unsigned cApiEventMask): cApiEventId_{cApiEventId}, cApiEventMask_{cApiEventMask} {}

  public:

    static const EventType TRADE;
    static const EventType QUOTE;
    static const EventType SUMMARY;
    static const EventType PROFILE;
    static const EventType ORDER;
    static const EventType TIME_AND_SALE;
    static const EventType CANDLE;
    static const EventType TRADE_ETH;
    static const EventType SPREAD_ORDER;
    static const EventType GREEKS;
    static const EventType THEO_PRICE;
    static const EventType UNDERLYING;
    static const EventType SERIES;
    static const EventType CONFIGURATION;
    static const EventType UNKNOWN;

    DXFCPP_CONSTEXPR unsigned getCApiEventId() const {
        return cApiEventId_;
    }

    DXFCPP_CONSTEXPR unsigned getCApiEventMask() const {
        return cApiEventMask_;
    }
};

const EventType EventType::TRADE{dx_eid_trade, DXF_ET_TRADE};
const EventType EventType::QUOTE{dx_eid_quote, DXF_ET_QUOTE};
const EventType EventType::SUMMARY{dx_eid_summary, DXF_ET_SUMMARY};
const EventType EventType::PROFILE{dx_eid_profile, DXF_ET_PROFILE};
const EventType EventType::ORDER{dx_eid_order, DXF_ET_ORDER};
const EventType EventType::TIME_AND_SALE{dx_eid_time_and_sale, DXF_ET_TIME_AND_SALE};
const EventType EventType::CANDLE{dx_eid_candle, DXF_ET_CANDLE};
const EventType EventType::TRADE_ETH{dx_eid_trade_eth, DXF_ET_TRADE_ETH};
const EventType EventType::SPREAD_ORDER{dx_eid_spread_order, DXF_ET_SPREAD_ORDER};
const EventType EventType::GREEKS{dx_eid_greeks, DXF_ET_GREEKS};
const EventType EventType::THEO_PRICE{dx_eid_theo_price, DXF_ET_THEO_PRICE};
const EventType EventType::UNDERLYING{dx_eid_underlying, DXF_ET_UNDERLYING};
const EventType EventType::SERIES{dx_eid_series, DXF_ET_SERIES};
const EventType EventType::CONFIGURATION{dx_eid_configuration, DXF_ET_CONFIGURATION};
const EventType EventType::UNKNOWN{unsigned(-1), unsigned(-1)};

class EventTypesMask final {
    unsigned mask_;

  public:

    explicit EventTypesMask(unsigned mask) : mask_{mask} {}

    explicit EventTypesMask(const std::unordered_set<EventType>& eventTypes) {
        unsigned resultMask = 0u;

        for (const auto& eventType : eventTypes) {
            resultMask |= eventType.getCApiEventMask();
        }

        mask_ = resultMask;
    }

    DXFCPP_CONSTEXPR unsigned getMask() const {
        return mask_;
    }

    friend EventTypesMask operator | (const EventTypesMask& eventTypesMask, const EventType& eventType) {
        return EventTypesMask{eventTypesMask.mask_ | eventType.getCApiEventMask()};
    }

    friend EventTypesMask operator | (const EventType& eventType1, const EventType& eventType2) {
        return EventTypesMask{eventType1.getCApiEventMask() | eventType2.getCApiEventMask()};
    }
};

} // namespace dxfcpp

//namespace std {
//    template<>
//    struct hash<dxfcpp::EventType> {
//        std::size_t operator()(const dxfcpp::EventType& eventType) const noexcept {
//            return eventType.getCApiEventId();
//        }
//    };
//}