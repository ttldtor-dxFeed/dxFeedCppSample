#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include "common/DXFCppConfig.hpp"

#include <numeric>
#include <unordered_set>
#include <utility>

namespace dxfcpp {

/// The enumeration type that provides additional information about the dxFeed C++-API event type
/// Useful when used as a subscription parameter.
class EventType {
    /// The dxFeed C-API event id corresponding to the current enum object.
    unsigned cApiEventId_;

    /// The dxFeed C-API event mask corresponding to the current enum object.
    unsigned cApiEventMask_;

    // A flag that indicates that the current enum object is characterizing the Lasting (TICKER) event.
    bool isLasting_;

    // A flag that indicates that the current enum object is characterizing the Indexed event.
    bool isIndexed_;

    // A flag that indicates that the current enum object is characterizing the TimeSeries (HISTORY) event.
    bool isTimeSeries_;

    // A flag that indicates that the current enum object is characterizing the OnlyIndexed (Indexed, but not
    // TimeSeries) event.
    bool isOnlyIndexed_;

    /**
     * Creates the new enum object by event parameters
     *
     * @param cApiEventId The dxFeed C-API event id
     * @param cApiEventMask The dxFeed C-API event  mask
     * @param isLasting The flag that indicates that the current enum object is characterizing the Lasting (TICKER)
     * event.
     * @param isIndexed The flag that indicates that the current enum object is characterizing the Indexed event.
     * @param isTimeSeries The flag that indicates that the current enum object is characterizing the Lasting (HISTORY)
     * event.
     */
    EventType(unsigned cApiEventId, unsigned cApiEventMask, bool isLasting, bool isIndexed = false,
              bool isTimeSeries = false)
        : cApiEventId_{cApiEventId}, cApiEventMask_{cApiEventMask}, isLasting_{isLasting}, isIndexed_{isIndexed ||
                                                                                                      isTimeSeries},
          isTimeSeries_{isTimeSeries}, isOnlyIndexed_{isIndexed && !isTimeSeries} {}

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

    explicit EventType() : EventType(unsigned(-1), unsigned(-1), false) {}

    ///
    unsigned getCApiEventId() const { return cApiEventId_; }

    ///
    unsigned getCApiEventMask() const { return cApiEventMask_; }

    bool operator==(const EventType &eventType) const { return cApiEventId_ == eventType.cApiEventId_; }

    ///
    bool isLasting() const { return isLasting_; }
    ///
    bool isIndexed() const { return isIndexed_; }
    ///
    bool isTimeSeries() const { return isTimeSeries_; }
    ///
    bool isOnlyIndexed() const { return isOnlyIndexed_; }
};

const EventType EventType::TRADE{dx_eid_trade, DXF_ET_TRADE, true};
const EventType EventType::QUOTE{dx_eid_quote, DXF_ET_QUOTE, true};
const EventType EventType::SUMMARY{dx_eid_summary, DXF_ET_SUMMARY, true};
const EventType EventType::PROFILE{dx_eid_profile, DXF_ET_PROFILE, true};
const EventType EventType::ORDER{dx_eid_order, DXF_ET_ORDER, false, true};
const EventType EventType::TIME_AND_SALE{dx_eid_time_and_sale, DXF_ET_TIME_AND_SALE, false, true, true};
const EventType EventType::CANDLE{dx_eid_candle, DXF_ET_CANDLE, true, true, true};
const EventType EventType::TRADE_ETH{dx_eid_trade_eth, DXF_ET_TRADE_ETH, true};
const EventType EventType::SPREAD_ORDER{dx_eid_spread_order, DXF_ET_SPREAD_ORDER, false, true};
const EventType EventType::GREEKS{dx_eid_greeks, DXF_ET_GREEKS, true, true, true};
const EventType EventType::THEO_PRICE{dx_eid_theo_price, DXF_ET_THEO_PRICE, true, true, true};
const EventType EventType::UNDERLYING{dx_eid_underlying, DXF_ET_UNDERLYING, true, true, true};
const EventType EventType::SERIES{dx_eid_series, DXF_ET_SERIES, false, true};
const EventType EventType::CONFIGURATION{dx_eid_configuration, DXF_ET_CONFIGURATION, true};
const EventType EventType::UNKNOWN{};

} // namespace dxfcpp

namespace std {

template <> struct hash<dxfcpp::EventType> {
    std::size_t operator()(const dxfcpp::EventType &eventType) const noexcept { return eventType.getCApiEventId(); }
};

} // namespace std

namespace dxfcpp {

// The wrapper around the event type mask (enum) and operators.
class EventTypesMask final {
    unsigned mask_;

  public:
    static const EventTypesMask LASTING;
    static const EventTypesMask INDEXED;
    static const EventTypesMask TIME_SERIES;
    static const EventTypesMask ONLY_INDEXED;

    explicit EventTypesMask(unsigned mask) : mask_{mask} {}

    template <typename EventTypeIt> EventTypesMask(EventTypeIt begin, EventTypeIt end) {
        mask_ = std::accumulate(begin, end, 0u,
                                [](unsigned mask, const EventType &flag) { return mask | flag.getCApiEventMask(); });
    }

    EventTypesMask(std::initializer_list<EventType> eventTypes)
        : EventTypesMask(eventTypes.begin(), eventTypes.end()) {}

    DXFCPP_CONSTEXPR unsigned getMask() const { return mask_; }
};

EventTypesMask operator|(const EventTypesMask &eventTypesMask, const EventType &eventType) {
    return EventTypesMask{eventTypesMask.getMask() | eventType.getCApiEventMask()};
}

EventTypesMask operator|(const EventTypesMask &eventTypesMask, const EventTypesMask &eventTypesMask2) {
    return EventTypesMask{eventTypesMask.getMask() | eventTypesMask2.getMask()};
}

EventTypesMask operator|(const EventType &eventType1, const EventType &eventType2) {
    return EventTypesMask{eventType1.getCApiEventMask() | eventType2.getCApiEventMask()};
}

EventTypesMask operator&(const EventTypesMask &eventTypesMask, const EventType &eventType) {
    return EventTypesMask{eventTypesMask.getMask() & eventType.getCApiEventMask()};
}

EventTypesMask operator&(const EventTypesMask &eventTypesMask, const EventTypesMask &eventTypesMask2) {
    return EventTypesMask{eventTypesMask.getMask() & eventTypesMask2.getMask()};
}

EventTypesMask operator&(const EventType &eventType1, const EventType &eventType2) {
    return EventTypesMask{eventType1.getCApiEventMask() & eventType2.getCApiEventMask()};
}

const EventTypesMask EventTypesMask::LASTING{EventType::TRADE | EventType::QUOTE | EventType::SUMMARY |
                                             EventType::PROFILE | EventType::CANDLE | EventType::TRADE_ETH |
                                             EventType::GREEKS | EventType::THEO_PRICE | EventType::UNDERLYING |
                                             EventType::CONFIGURATION};
const EventTypesMask EventTypesMask::INDEXED{EventType::ORDER | EventType::TIME_AND_SALE | EventType::CANDLE |
                                             EventType::SPREAD_ORDER | EventType::GREEKS | EventType::THEO_PRICE |
                                             EventType::UNDERLYING | EventType::SERIES};
const EventTypesMask EventTypesMask::TIME_SERIES{EventType::TIME_AND_SALE | EventType::CANDLE | EventType::GREEKS |
                                                 EventType::THEO_PRICE | EventType::UNDERLYING};
const EventTypesMask EventTypesMask::ONLY_INDEXED{EventType::ORDER | EventType::SPREAD_ORDER | EventType::SERIES};

} // namespace dxfcpp
