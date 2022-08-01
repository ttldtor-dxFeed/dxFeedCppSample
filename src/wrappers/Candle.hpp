#pragma once

#include "EventData.h"
#include <string>
#include <utility>

#include "EventFlag.hpp"
#include "converters/DateTimeConverter.hpp"

namespace dxfcs {

/**
 * Wrapper over the "Candle" event + additional traits for getting meta-information to interact with the native API.
 * + symbol + conversion to string and some getters.
 */
struct Candle {
    // dxFeed Event Type "traits"
    using RawType = dxf_candle_t;
    static const unsigned EVENT_ID = dx_eid_candle;
    static const unsigned EVENT_TYPE = DXF_ET_CANDLE;

    using IndexType = decltype(RawType::index);
    using EventFlagsType = unsigned;
    using TimeType = decltype(RawType::time);

  private:
    std::string symbol_;
    RawType data_;

  public:
    Candle(std::string symbol, RawType const &candle) : symbol_{std::move(symbol)}, data_{candle} {}

    const std::string &getSymbol() const { return symbol_; }

    const RawType &getData() const { return data_; }

    EventFlagsType getEventFlags() const { return data_.event_flags; }

    void setEventFlags(EventFlagsType flags) { data_.event_flags = flags; }

    IndexType getIndex() const { return data_.index; }

    TimeType getTime() const { return data_.time; }

    std::string toString() const {
        return std::string("Candle{") + symbol_ + ", index=" + std::to_string(getIndex()) +
            ", eventTime=" + DateTimeConverter::toISO(getTime()) + ", eventFlags=" + std::to_string(getEventFlags()) +
            ", sequence=" + std::to_string(data_.sequence) + ", count=" + std::to_string(data_.count) +
            ", open=" + std::to_string(data_.open) + ", high=" + std::to_string(data_.high) +
            ", low=" + std::to_string(data_.low) + ", close=" + std::to_string(data_.close) +
            ", volume=" + std::to_string(data_.volume) + ", vwap=" + std::to_string(data_.vwap) +
            ", bidVolume=" + std::to_string(data_.bid_volume) + ", askVolume=" + std::to_string(data_.ask_volume) +
            ", impVolatility=" + std::to_string(data_.imp_volatility) +
            ", openInterest=" + std::to_string(data_.open_interest) + "}";
    }
};
} // namespace dxfcs