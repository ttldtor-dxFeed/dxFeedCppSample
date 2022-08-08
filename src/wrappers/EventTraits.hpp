#pragma once

#include <limits>
#include <type_traits>

#include "DXFCppConfig.hpp"

#include <DXFeed.h>
#include <DXTypes.h>
#include <EventData.h>
#include "EventType.hpp"

namespace dxfcpp {

struct UnknownCApiEventType {};
struct UnknownEventType {};

struct EventTraitsBase {
    using Type = UnknownEventType;

    static EventType getEventType() {
        return EventType::UNKNOWN;
    }

    static DXFCPP_USE_CONSTEXPR bool isSpecialized = false;
    using CApiEventType = UnknownCApiEventType;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventId = 0u;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventMask = 0u;
    static DXFCPP_USE_CONSTEXPR bool isMarketEvent = false;
    static DXFCPP_USE_CONSTEXPR bool isIndexedEvent = false;
    static DXFCPP_USE_CONSTEXPR bool isLastingEvent = false;
    static DXFCPP_USE_CONSTEXPR bool isTimeSeriesEvent = false;
    static DXFCPP_USE_CONSTEXPR bool isOnlyIndexedEvent = false;
};

template <typename T> struct EventTraits : public EventTraitsBase {
};

template<typename T>
struct EventTraits<const T>
    : public EventTraits<T> { };

template<typename T>
struct EventTraits<volatile T>
    : public EventTraits<T> { };

template<typename T>
struct EventTraits<const volatile T>
    : public EventTraits<T> { };

struct Quote;
struct Candle;

template<>
struct EventTraits<Quote> : public EventTraitsBase {
    using Type = Quote;

    static EventType getEventType() {
        return EventType::QUOTE;
    }

    static DXFCPP_USE_CONSTEXPR bool isSpecialized = true;
    using CApiEventType = dxf_quote_t;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventId = dx_eid_quote;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventMask = DXF_ET_QUOTE;
    static DXFCPP_USE_CONSTEXPR bool isMarketEvent = true;
    static DXFCPP_USE_CONSTEXPR bool isLastingEvent = true;
};

template<>
struct EventTraits<Candle> : public EventTraitsBase {
    using Type = Candle;

    static EventType getEventType() {
        return EventType::CANDLE;
    }

    static DXFCPP_USE_CONSTEXPR bool isSpecialized = true;
    using CApiEventType = dxf_candle_t;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventId = dx_eid_candle;
    static DXFCPP_USE_CONSTEXPR unsigned cApiEventMask = DXF_ET_CANDLE;
    static DXFCPP_USE_CONSTEXPR bool isIndexedEvent = true;
    static DXFCPP_USE_CONSTEXPR bool isLastingEvent = true;
    static DXFCPP_USE_CONSTEXPR bool isTimeSeriesEvent = true;
};

template<unsigned cApiEventMask>
struct CApiEventMaskToEvent {
    using Type = UnknownEventType;
};

template<>
struct CApiEventMaskToEvent<DXF_ET_QUOTE> {
    using Type = Quote;
};

template<>
struct CApiEventMaskToEvent<DXF_ET_CANDLE> {
    using Type = Candle;
};

} // namespace dxfcpp