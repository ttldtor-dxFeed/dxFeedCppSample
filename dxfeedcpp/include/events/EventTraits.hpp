#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include <limits>
#include <type_traits>

extern "C" {
#include <DXFeed.h>
#include <DXTypes.h>
#include <EventData.h>
}

#include "common/DXFCppConfig.hpp"

#include "EventType.hpp"

namespace dxfcpp {

/// Stub for an unknown dxFeed C-API type that matches the current type (T)
struct UnknownCApiEventType {};
/// Stub for an unknown dxFeed C++-API type that matches the current type (T)
struct UnknownEventType {};

/// Base helper type that provides additional compile-time information about the dxFeed C++-API event type
struct EventTraitsBase {
    /// The current type stub for all non-event types
    using Type = UnknownEventType;

    /// The EventType "enum" stub for all non-event types
    static EventType getEventType() { return EventType::UNKNOWN; }

    /// The specialization flag's stub for all non-event types
    static DXFCPP_USE_CONSTEXPR bool isSpecialized = false;

    /// The dxFeed C-API event stub for all non-event types
    using CApiEventType = UnknownCApiEventType;

    /// The dxFeed C-API event id stub for all non-event types
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventId = 0u;

    /// The dxFeed C-API event mask stub for all non-event types
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventMask = 0u;

    /// The market event flag's stub for all non-event types
    static DXFCPP_USE_CONSTEXPR bool isMarketEvent = false;

    /// The indexed event flag's stub for all non-event types
    static DXFCPP_USE_CONSTEXPR bool isIndexedEvent = false;

    /// The lasting event flag's stub for all non-event types
    static DXFCPP_USE_CONSTEXPR bool isLastingEvent = false;

    /// The time series event flag's stub for all non-event types
    static DXFCPP_USE_CONSTEXPR bool isTimeSeriesEvent = false;

    /// The only indexed (indexed, but not time series) event flag's stub for all non-event types
    static DXFCPP_USE_CONSTEXPR bool isOnlyIndexedEvent = false;
};

/**
 * A basic typed helper that provides additional compile-time information about the type (presumably the dxFeed C++
 * event)
 *
 * @tparam T The type for which additional information is provided.
 */
template <typename T> struct EventTraits : public EventTraitsBase {};

/**
 * A basic typed helper that provides additional compile-time information about the type (presumably the dxFeed C++
 * event)
 *
 * @tparam T The type for which additional information is provided.
 */
template <typename T> struct EventTraits<const T> : public EventTraits<T> {};

/**
 * A basic typed helper that provides additional compile-time information about the type (presumably the dxFeed C++
 * event)
 *
 * @tparam T The type for which additional information is provided.
 */
template <typename T> struct EventTraits<volatile T> : public EventTraits<T> {};

/**
 * A basic typed helper that provides additional compile-time information about the type (presumably the dxFeed C++
 * event)
 *
 * @tparam T The type for which additional information is provided.
 */
template <typename T> struct EventTraits<const volatile T> : public EventTraits<T> {};

struct Quote;
struct Candle;
struct Trade;
struct TradeETH;
// TODO: add events

/**
 * A typed helper that provides additional compile-time information about the Quote type
 */
template <> struct EventTraits<Quote> : public EventTraitsBase {
    // The current type is Quote
    using Type = Quote;

    // The current EventType enum
    static EventType getEventType() { return EventType::QUOTE; }

    // The current type is specialized
    static DXFCPP_USE_CONSTEXPR bool isSpecialized = true;

    // The current dxFeed C-API event type
    using CApiEventType = dxf_quote_t;

    // The current dxFeed C-API event id (enum)
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventId = dx_eid_quote;

    // The current dxFeed C-API event mask
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventMask = DXF_ET_QUOTE;

    // The current event is a market event
    static DXFCPP_USE_CONSTEXPR bool isMarketEvent = true;

    // The current event is a lasting event
    static DXFCPP_USE_CONSTEXPR bool isLastingEvent = true;
};

/**
 * A typed helper that provides additional compile-time information about the Candle type
 */
template <> struct EventTraits<Candle> : public EventTraitsBase {
    // The current type is Candle
    using Type = Candle;

    // The current EventType enum
    static EventType getEventType() { return EventType::CANDLE; }

    // The current type is specialized
    static DXFCPP_USE_CONSTEXPR bool isSpecialized = true;

    // The current dxFeed C-API event type
    using CApiEventType = dxf_candle_t;

    // The current dxFeed C-API event id (enum)
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventId = dx_eid_candle;

    // The current dxFeed C-API event mask
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventMask = DXF_ET_CANDLE;

    // The current event is an indexed event
    static DXFCPP_USE_CONSTEXPR bool isIndexedEvent = true;

    // The current event is a lasting event
    static DXFCPP_USE_CONSTEXPR bool isLastingEvent = true;

    // The current event is a time series event
    static DXFCPP_USE_CONSTEXPR bool isTimeSeriesEvent = true;
};

/**
 * A typed helper that provides additional compile-time information about the Trade type
 */
template <> struct EventTraits<Trade> : public EventTraitsBase {
    using Type = Trade;

    static EventType getEventType() { return EventType::TRADE; }
    static DXFCPP_USE_CONSTEXPR bool isSpecialized = true;
    using CApiEventType = dxf_trade_t;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventId = dx_eid_trade;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventMask = DXF_ET_TRADE;
    static DXFCPP_USE_CONSTEXPR bool isMarketEvent = true;
    static DXFCPP_USE_CONSTEXPR bool isLastingEvent = true;
};

/**
 * A typed helper that provides additional compile-time information about the TradeETH type
 */
template <> struct EventTraits<TradeETH> : public EventTraitsBase {
    using Type = TradeETH;

    static EventType getEventType() { return EventType::TRADE_ETH; }
    static DXFCPP_USE_CONSTEXPR bool isSpecialized = true;
    using CApiEventType = dxf_trade_eth_t;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventId = dx_eid_trade_eth;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventMask = DXF_ET_TRADE_ETH;
    static DXFCPP_USE_CONSTEXPR bool isMarketEvent = true;
    static DXFCPP_USE_CONSTEXPR bool isLastingEvent = true;
};

} // namespace dxfcpp