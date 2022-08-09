#pragma once

#include "EventData.h"
#include <string>
#include <utility>

#include "DXFCppConfig.hpp"

#include "EventFlags.hpp"
#include "EventTraits.hpp"
#include "OrderScope.hpp"
#include "converters/DateTimeConverter.hpp"
#include "converters/StringConverter.hpp"

namespace dxfcpp {

/**
 * Wrapper over the "Quote" event + additional traits for getting meta-information to interact with the native API.
 * + symbol + conversion to string and some getters.
 */
struct Quote {
    using Ptr = std::shared_ptr<Quote>;
    using CApiEventType = EventTraits<Quote>::CApiEventType;
    using TimeType = decltype(CApiEventType::time);

  private:
    std::string symbol_;
    CApiEventType data_;

  public:
    Quote(std::string symbol, CApiEventType const &quote) : symbol_{std::move(symbol)}, data_{quote} {}

    const std::string &getSymbol() const { return symbol_; }

    const CApiEventType &getData() const { return data_; }

    TimeType getTime() const { return data_.time; }

    std::string toString() const {
        return std::string("Quote{") + symbol_ + ", eventTime=" + DateTimeConverter::toISO(getTime()) +
            ", timeNanoPart=" + std::to_string(data_.time_nanos) + ", sequence=" + std::to_string(data_.sequence) +
            ", bidTime=" + DateTimeConverter::toISO(data_.bid_time) +
            ", bidExchange=" + StringConverter::wCharToUtf8(data_.bid_exchange_code) +
            ", bidPrice=" + std::to_string(data_.bid_price) + ", bidSize=" + std::to_string(data_.bid_size) +
            ", askTime=" + DateTimeConverter::toISO(data_.ask_time) +
            ", askExchange=" + StringConverter::wCharToUtf8(data_.ask_exchange_code) +
            ", askPrice=" + std::to_string(data_.ask_price) + ", askSize=" + std::to_string(data_.ask_size) +
            ", scope=" + OrderScope::get(data_.scope).toString() + "}";
    }

    template <class Ostream> friend Ostream &&operator<<(Ostream &&os, const Quote &value) {
        return std::forward<Ostream>(os) << value.toString();
    }
};

} // namespace dxfcpp