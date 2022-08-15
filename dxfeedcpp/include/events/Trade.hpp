#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

extern "C" {
#include <EventData.h>
}

#include <cmath>
#include <string>
#include <utility>

#include "common/DXFCppConfig.hpp"

#include "converters/DateTimeConverter.hpp"
#include "converters/StringConverter.hpp"

#include "Direction.hpp"
#include "Event.hpp"
#include "EventFlags.hpp"
#include "EventTraits.hpp"

namespace dxfcpp {

struct TradeBase : virtual public MarketEvent, virtual public Lasting {
    using Ptr = std::shared_ptr<TradeBase>;

  private:
    /// Time of the last trade.
    std::uint64_t time_{};
    /// Sequence number of the last trade to distinguish trades that have the same #time_.
    std::int32_t sequence_{};
    /// Microseconds and nanoseconds part of time of the last trade
    std::int32_t timeNanoPart_{};
    /// Exchange code of the last trade
    char exchangeCode_{};
    /// Price of the last trade
    double price_{std::numeric_limits<double>::quiet_NaN()};
    /// Size of the last trade
    double size_{std::numeric_limits<double>::quiet_NaN()};

    /**
     * Trend indicator – in which direction price is moving. The values are: Up (Tick = 1), Down (Tick = 2),
     * and Undefined (Tick = 0).
     * Should be used if #direction_ is Undefined (Direction#UDEFINED = 0).
     *
     * This field is absent in TradeETH
     */
    std::int32_t tick_{};
    /**
     * Change of the last trade.
     * Value equals price minus dxf_summary_t#prev_day_close_price
     */
    double change_{};
    /// Identifier of the day that this `Trade` or `TradeETH` represents. Identifier of the day is the number of days
    /// passed since January 1, 1970.
    std::int32_t dayId_{};
    /// Total volume traded for a day
    double dayVolume_{std::numeric_limits<double>::quiet_NaN()};
    /// Total turnover traded for a day
    double dayTurnover_{std::numeric_limits<double>::quiet_NaN()};
    /**
     * This field contains several individual flags encoded as an integer number the following way:
     * <table>
     * <tr><th>31...4</th><th>3</th><th>2</th><th>1</th><th>0</th></tr>
     * <tr><td></td><td colspan="3">Direction</td><td>ETH</td></tr>
     * </table>
     *
     * 1. Tick Direction (#Direction)
     * 2. ETH (extendedTradingHours) - flag that determines current trading session: extended or regular (0 - regular
     *    trading hours, 1 - extended trading hours).
     */
    unsigned rawFlags_{};
    /// Tick direction of the last trade
    Direction direction_;
    /// Last trade was in extended trading hours
    bool isETH_{};
    /**
     * Last trade scope.
     *
     * Possible values: OrderScope::COMPOSITE (Trade events) , OrderScope::REGIONAL (Trade& events)
     */
    OrderScope scope_;

  public:
    TradeBase(std::string eventSymbol, const dxf_trade_t &trade)
        : MarketEvent(std::move(eventSymbol)), time_{static_cast<std::uint64_t>(trade.time)}, sequence_{trade.sequence},
          timeNanoPart_{trade.time_nanos}, exchangeCode_{StringConverter::wCharToUtf8(trade.exchange_code)},
          price_{trade.price}, size_{trade.size}, tick_{trade.tick}, change_{trade.change}, dayId_{trade.day_id},
          dayVolume_{trade.day_volume}, dayTurnover_{trade.day_turnover}, rawFlags_{static_cast<unsigned>(
                                                                              trade.raw_flags)},
          direction_{Direction::get(trade.direction)}, isETH_{static_cast<bool>(trade.is_eth)}, scope_{OrderScope::get(
                                                                                                    trade.scope)} {
        if (direction_ == Direction::UNDEFINED) {
            if (tick_ == 1) {
                direction_ = Direction::ZERO_UP;
            } else if (tick_ == 2) {
                direction_ = Direction::ZERO_DOWN;
            }
        }
    }

    TradeBase() : MarketEvent(), direction_(Direction::UNDEFINED), scope_(OrderScope::UNKNOWN) {}
    TradeBase &operator=(const TradeBase &other) = delete;
    TradeBase &operator=(TradeBase &&other) = delete;
    TradeBase(const TradeBase &other)
        : MarketEvent(other), time_{other.time_}, sequence_{other.sequence_}, timeNanoPart_{other.timeNanoPart_},
          exchangeCode_{other.exchangeCode_}, price_{other.price_}, size_{other.size_}, tick_{other.tick_},
          change_{other.change_}, dayId_{other.dayId_}, dayVolume_{other.dayVolume_}, dayTurnover_{other.dayTurnover_},
          rawFlags_{other.rawFlags_}, direction_{other.direction_}, isETH_{other.isETH_}, scope_{other.scope_} {}
    TradeBase(TradeBase &&other) noexcept
        : MarketEvent(std::move(other)), time_{other.time_}, sequence_{other.sequence_},
          timeNanoPart_{other.timeNanoPart_}, exchangeCode_{other.exchangeCode_}, price_{other.price_},
          size_{other.size_}, tick_{other.tick_}, change_{other.change_}, dayId_{other.dayId_},
          dayVolume_{other.dayVolume_}, dayTurnover_{other.dayTurnover_}, rawFlags_{other.rawFlags_},
          direction_{std::move(other.direction_)}, isETH_{other.isETH_}, scope_{std::move(other.scope_)} {}
    explicit TradeBase(std::string eventSymbol)
        : MarketEvent(std::move(eventSymbol)), direction_(Direction::UNDEFINED), scope_(OrderScope::UNKNOWN) {}
    ~TradeBase() override = default;

    // TODO: utils
    /**
     * Returns quotient according to number theory - i.e. when remainder is zero or positive.
     *
     * @param a dividend
     * @param b divisor
     * @return quotient according to number theory
     */
    static std::int32_t div(std::int32_t a, std::int32_t b) {
        return a >= 0 ? a / b : b >= 0 ? (a + 1) / b - 1 : (a + 1) / b + 1;
    }

    static std::int32_t getYearMonthDayByDayId(std::int32_t dayId) {
        std::int32_t j = dayId + 2472632; // this shifts the epoch back to astronomical year -4800
        std::int32_t g = div(j, 146097);
        std::int32_t dg = j - g * 146097;
        std::int32_t c = (dg / 36524 + 1) * 3 / 4;
        std::int32_t dc = dg - c * 36524;
        std::int32_t b = dc / 1461;
        std::int32_t db = dc - b * 1461;
        std::int32_t a = (db / 365 + 1) * 3 / 4;
        std::int32_t da = db - a * 365;
        std::int32_t y = g * 400 + c * 100 + b * 4 +
            a; // this is the integer number of full years elapsed since March 1, 4801 BC at 00:00 UTC
        std::int32_t m = (da * 5 + 308) / 153 -
            2; // this is the integer number of full months elapsed since the last March 1 at 00:00 UTC
        std::int32_t d =
            da - (m + 4) * 153 / 5 + 122; // this is the number of days elapsed since day 1 of the month at 00:00 UTC
        std::int32_t yyyy = y - 4800 + (m + 2) / 12;
        std::int32_t mm = (m + 2) % 12 + 1;
        std::int32_t dd = d + 1;
        std::int32_t yyyymmdd = std::abs(yyyy) * 10000 + mm * 100 + dd;
        return yyyy >= 0 ? yyyymmdd : -yyyymmdd;
    }

    std::string baseFieldsToString() const {
        return getEventSymbol() + ", eventTime=" + DateTimeConverter::toISO(getEventTime()) +
            ", time=" + DateTimeConverter::toISO(time_) + ", sequence=" + std::to_string(sequence_) +
            ", timeNanoPart=" + std::to_string(timeNanoPart_) + ", exchange=" + exchangeCodeToString(exchangeCode_) +
            ", price=" + std::to_string(price_) + ", size=" + std::to_string(size_) +
            ", tick=" + std::to_string(tick_) + ", change=" + std::to_string(change_) +
            ", day=" + std::to_string(getYearMonthDayByDayId(dayId_)) + ", dayVolume=" + std::to_string(dayVolume_) +
            ", dayTurnover=" + std::to_string(dayTurnover_) + ", direction=" + direction_.toString() +
            ", ETH=" + std::to_string(isETH_);
    }
};

struct Trade final : virtual public TradeBase {
    Trade(std::string eventSymbol, const dxf_trade_t &trade) {}
    std::string toString() const override { return std::string(); }
};

struct TradeETH final : virtual public TradeBase {
    TradeETH(std::string eventSymbol, const dxf_trade_eth_t &trade) {}
    std::string toString() const override { return std::string(); }
};

} // namespace dxfcpp