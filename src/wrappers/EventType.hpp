#pragma once

namespace dxfcpp {

enum class EventType {
    Trade,
    Quote,
    Summary,
    Profile,
    Order,
    TimeAndSale,
    Candle,
    TradeEth,
    SpreadOrder,
    Greeks,
    TheoPrice,
    Underlying,
    Series,
    Configuration,
    Unknown = 255,
};

template <EventType eventType> struct EventTypeEnumToEventType {};

struct Quote;
struct Candle;

template <> struct EventTypeEnumToEventType<EventType::Quote> {
    using Type = Quote;
};

template <> struct EventTypeEnumToEventType<EventType::Candle> {
    using Type = Candle;
};

} // namespace dxfcpp