#pragma once

#include "EventData.h"
#include <string>
#include <utility>

#include "EventFlag.hpp"
#include "OrderScope.hpp"
#include "converters/DateTimeConverter.hpp"
#include "converters/StringConverter.hpp"

namespace dxfcs {

struct Quote {
    // dxFeed Event Type "traits"
    using RawType = dxf_quote_t;
    static const unsigned EVENT_ID = dx_eid_quote;
    static const unsigned EVENT_TYPE = DXF_ET_QUOTE;

    using TimeType = decltype(RawType::time);

  private:
    std::string symbol_;
    RawType data_;

  public:
    Quote(std::string symbol, RawType const &quote) : symbol_{std::move(symbol)}, data_{quote} {}

    const std::string &getSymbol() const { return symbol_; }

    const RawType &getData() const { return data_; }

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
};

} // namespace dxfcs