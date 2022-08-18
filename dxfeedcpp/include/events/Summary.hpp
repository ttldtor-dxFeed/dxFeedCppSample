#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

extern "C" {
#include <EventData.h>
}

#include <string>
#include <utility>

#include "common/DXFCppConfig.hpp"

#include "converters/DateTimeConverter.hpp"

#include "Event.hpp"
#include "EventFlags.hpp"
#include "EventTraits.hpp"
#include "PriceType.hpp"

namespace dxfcpp {

struct Summary final : public MarketEvent, public Lasting {
    /// Identifier of the day that this summary represents. Identifier of the day is the number of days passed since
    /// January 1, 1970.
    std::int32_t dayId_{};

    /// The first (open) price for the day.
    double dayOpenPrice_{std::numeric_limits<double>::quiet_NaN()};

    /// The maximal (high) price for the day
    double dayHighPrice_{std::numeric_limits<double>::quiet_NaN()};

    /// The minimal (low) price for the day
    double dayLowPrice_{std::numeric_limits<double>::quiet_NaN()};

    /// The last (close) price for the day
    double dayClosePrice_{std::numeric_limits<double>::quiet_NaN()};

    /// Identifier of the previous day that this summary represents. Identifier of the day is the number of days passed
    /// since January 1, 1970.
    std::int32_t prevDayId_{};

    /// The last (close) price for the previous day
    double prevDayClosePrice_{std::numeric_limits<double>::quiet_NaN()};

    /// Total volume traded for the previous day
    double prevDayVolume_{std::numeric_limits<double>::quiet_NaN()};

    /// Open interest of the symbol as the number of open contracts
    double openInterest_{std::numeric_limits<double>::quiet_NaN()};

    /**
     * This field contains several individual flags encoded as an integer number the following way:
     * <table>
     * <tr><th>31...4</th><th>3</th><th>2</th><th>1</th><th>0</th></tr>
     * <tr><td></td><td colspan="2">Close</td><td colspan="2">PrevClose</td></tr>
     * </table>
     *
     * 1. Close (dayClosePriceType) - parameter that shows if the closing price is final PriceType
     * 2. PrevClose (prevDayClosePriceType) - parameter that shows if the closing price of the previous day is final
     * PriceType
     */
    std::uint32_t rawFlags_{};

    /// Exchange code
    char exchangeCode_;

    /// The price type of the last (close) price for the day
    PriceType dayClosePriceType_;

    /// The price type of the last (close) price for the previous day
    PriceType prevDayClosePriceType_;

    /**
     * Scope of this summary.
     *
     * Possible values: OrderScope#COMPOSITE (Summary events) , OrderScope#REGIONAL (Summary& events)
     */
    OrderScope scope_;

  public:
    /**
     * Creates new summary event with the specified event symbol and dxFeed C-API dxf_summary_t
     *
     * @param eventSymbol The event symbol
     * @param summary The dxFeed C-API dxf_summary_t object
     */
    Summary(std::string eventSymbol, const dxf_summary_t &summary)
        : MarketEvent(std::move(eventSymbol)), dayId_{summary.day_id}, dayOpenPrice_{summary.day_open_price},
          dayHighPrice_{summary.day_high_price}, dayLowPrice_{summary.day_low_price},
          dayClosePrice_{summary.day_close_price}, prevDayId_{summary.prev_day_id},
          prevDayClosePrice_{summary.prev_day_close_price}, prevDayVolume_{summary.prev_day_volume},
          openInterest_{summary.open_interest}, rawFlags_{static_cast<std::uint32_t>(summary.raw_flags)},
          exchangeCode_{StringConverter::wCharToUtf8(summary.exchange_code)}, dayClosePriceType_{PriceType::get(
                                                                                  summary.day_close_price_type)},
          prevDayClosePriceType_{PriceType::get(summary.prev_day_close_price_type)}, scope_{OrderScope::get(
                                                                                         summary.scope)} {}

    Summary &operator=(const Summary &other) = delete;

    Summary &operator=(Summary &&other) = delete;

    /// Returns sn identifier of the day that this summary represents. Identifier of the day is the number of days
    /// passed since January 1, 1970.
    int32_t getDayId() const { return dayId_; }

    /// Returns the first (open) price for the day.
    double getDayOpenPrice() const { return dayOpenPrice_; }

    /// Returns the maximal (high) price for the day
    double getDayHighPrice() const { return dayHighPrice_; }

    /// Returns the minimal (low) price for the day
    double getDayLowPrice() const { return dayLowPrice_; }

    /// Returns the last (close) price for the day
    double getDayClosePrice() const { return dayClosePrice_; }

    /// Returns an identifier of the previous day that this summary represents. Identifier of the day is the number of
    /// days passed since January 1, 1970.
    int32_t getPrevDayId() const { return prevDayId_; }

    /// Returns the last (close) price for the previous day
    double getPrevDayClosePrice() const { return prevDayClosePrice_; }

    /// Returns the total volume traded for the previous day
    double getPrevDayVolume() const { return prevDayVolume_; }

    /// Returns the open interest of the symbol as the number of open contracts
    double getOpenInterest() const { return openInterest_; }

    /// Returns the summary's raw flags
    uint32_t getRawFlags() const { return rawFlags_; }

    /// Returns the summary's exchange code
    char getExchangeCode() const { return exchangeCode_; }

    /// Returns the price type of the last (close) price for the day
    const PriceType &getDayClosePriceType() const { return dayClosePriceType_; }

    /// Returns the price type of the last (close) price for the previous day
    const PriceType &getPrevDayClosePriceType() const { return prevDayClosePriceType_; }

    /**
     * Returns the scope of this summary.
     *
     * Possible values: OrderScope#COMPOSITE (Summary events) , OrderScope#REGIONAL (Summary& events)
     */
    const OrderScope &getScope() const { return scope_; }

    /// Returns a string representation of the entity
    std::string toString() const override {
        return "Summary{" + getEventSymbol() + ", eventTime=" + DateTimeConverter::toISO(getEventTime()) +
            ", day=" + std::to_string(day_util::getYearMonthDayByDayId(dayId_)) +
            ", dayOpen=" + std::to_string(dayOpenPrice_) + ", dayHigh=" + std::to_string(dayHighPrice_) +
            ", dayLow=" + std::to_string(dayLowPrice_) + ", dayClose=" + std::to_string(dayClosePrice_) +
            ", dayCloseType=" + dayClosePriceType_.toString() +
            ", prevDay=" + std::to_string(day_util::getYearMonthDayByDayId(prevDayId_)) +
            ", prevDayClose=" + std::to_string(prevDayClosePrice_) +
            ", prevDayCloseType=" + prevDayClosePriceType_.toString() +
            ", prevDayVolume=" + std::to_string(prevDayVolume_) + ", openInterest=" + std::to_string(openInterest_) +
            ", rawFlags=" + string::toHex(rawFlags_) + ", exchange=" + exchangeCodeToString(exchangeCode_) +
            ", scope=" + scope_.toString() + '}';
    }
};

} // namespace dxfcpp