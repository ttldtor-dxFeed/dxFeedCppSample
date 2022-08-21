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

#include "utils/Utils.hpp"

#include "Direction.hpp"
#include "Event.hpp"
#include "EventFlags.hpp"
#include "EventTraits.hpp"

namespace dxfcpp {

/**
 * Base class for common fields of #Trade and #TradeETH events.
 * Trade events represent the most recent information that is available about the last trade on the market
 * at any given moment of time.
 */
struct TradeBase : public MarketEvent, public Lasting {
    /// The alias to a type of shared pointer to the TradeBase object
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
     * Value equals price minus Summary#getPrevDayClosePrice()
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
    std::uint32_t rawFlags_{};

    /// Tick direction of the last trade
    Direction direction_;

    /// Last trade was in extended trading hours
    bool isEth_{};

    /**
     * Last trade scope.
     *
     * Possible values: OrderScope::COMPOSITE (Trade events) , OrderScope::REGIONAL (Trade& events)
     */
    OrderScope scope_;

  public:
    /**
     * Creates new trade with the specified event symbol and dxFeed C-API dxf_trade_t
     *
     * @param eventSymbol The event symbol
     * @param trade The dxFeed C-API dxf_trade_t object
     */
    TradeBase(std::string eventSymbol, const dxf_trade_t &trade)
        : MarketEvent(std::move(eventSymbol)), time_{static_cast<std::uint64_t>(trade.time)}, sequence_{trade.sequence},
          timeNanoPart_{trade.time_nanos}, exchangeCode_{StringConverter::wCharToUtf8(trade.exchange_code)},
          price_{trade.price}, size_{trade.size}, tick_{trade.tick}, change_{trade.change}, dayId_{trade.day_id},
          dayVolume_{trade.day_volume}, dayTurnover_{trade.day_turnover}, rawFlags_{static_cast<std::uint32_t>(
                                                                              trade.raw_flags)},
          direction_{Direction::get(trade.direction)}, isEth_{static_cast<bool>(trade.is_eth)}, scope_{OrderScope::get(
                                                                                                    trade.scope)} {
        // Tries to infer the value of direction from tick if direction is UNDEFINED.
        if (direction_ == Direction::UNDEFINED) {
            if (tick_ == 1) {
                direction_ = Direction::ZERO_UP;
            } else if (tick_ == 2) {
                direction_ = Direction::ZERO_DOWN;
            }
        }
    }

    TradeBase &operator=(const TradeBase &other) = delete;

    TradeBase &operator=(TradeBase &&other) = delete;

    /// Creates new Trade or TradeTH with specified event symbol
    explicit TradeBase(std::string eventSymbol)
        : MarketEvent(std::move(eventSymbol)), direction_(Direction::UNDEFINED), scope_(OrderScope::UNKNOWN) {}
    ~TradeBase() override = default;

    /// Returns the last trade time
    uint64_t getTime() const { return time_; }

    /// Returns the sequence number of the last trade (to distinguish trades that have the same #time_)
    int32_t getSequence() const { return sequence_; }

    /// Returns microseconds and nanoseconds part of time of the last trade
    int32_t getTimeNanoPart() const { return timeNanoPart_; }

    /// Returns an exchange code of the last trade
    char getExchangeCode() const { return exchangeCode_; }

    /// Returns a price of the last trade
    double getPrice() const { return price_; }

    /// Returns a size of the last trade
    double getSize() const { return size_; }

    /// Returns the trend indicator – in which direction price is moving. The values are: Up (Tick = 1),
    /// Down (Tick = 2) and Undefined (Tick = 0).
    int32_t getTick() const { return tick_; }

    /// Returns a change of the last trade.
    double getChange() const { return change_; }

    /// Returns an identifier of the day that this `Trade` or `TradeETH` represents.
    int32_t getDayId() const { return dayId_; }

    /// Returns a total volume traded for a day
    double getDayVolume() const { return dayVolume_; }

    /// Returns a total turnover traded for a day
    double getDayTurnover() const { return dayTurnover_; }

    /// Returns a raw flags of the last trade
    std::uint32_t getFlags() const { return rawFlags_; }

    /// Returns a tick direction of the last trade
    const Direction &getDirection() const { return direction_; }

    /// Returns true if the last trade was in extended trading hours
    bool isExtendedTradingHours() const { return isEth_; }

    /// Returns the last trade scope.
    /// Possible values: OrderScope::COMPOSITE (Trade events), OrderScope::REGIONAL (Trade& events)
    const OrderScope &getScope() const { return scope_; }

    /// Returns a string representation of the base fields
    std::string baseFieldsToString() const {
        return std::string() + getEventSymbol() + ", eventTime=" + DateTimeConverter::toISO(getEventTime()) +
            ", time=" + DateTimeConverter::toISO(time_) + ", sequence=" + std::to_string(sequence_) +
            ", timeNanoPart=" + std::to_string(timeNanoPart_) + ", exchange=" + exchangeCodeToString(exchangeCode_) +
            ", price=" + std::to_string(price_) + ", size=" + std::to_string(size_) +
            ", tick=" + std::to_string(tick_) + ", change=" + std::to_string(change_) +
            ", day=" + std::to_string(day_util::getYearMonthDayByDayId(dayId_)) +
            ", dayVolume=" + std::to_string(dayVolume_) + ", dayTurnover=" + std::to_string(dayTurnover_) +
            ", rawFlags=" + string::toHex(rawFlags_) + ", scope=" + scope_.toString() +
            ", direction=" + direction_.toString() + ", ETH=" + std::to_string(isEth_);
    }
};

/**
 * Trade event is a snapshot of the price and size of the last trade during regular trading hours
 * and an overall day volume and day turnover.
 * It represents the most recent information that is available about the regular last trade on the market
 * at any given moment of time.
 */
struct Trade final : public TradeBase {
    /// The alias to a type of shared pointer to the Trade object
    using Ptr = std::shared_ptr<Trade>;

    /**
     * Creates new trade with the specified event symbol and dxFeed C-API dxf_trade_t
     *
     * @param eventSymbol The event symbol
     * @param trade The dxFeed C-API dxf_trade_t object
     */
    Trade(std::string eventSymbol, const dxf_trade_t &trade) : TradeBase(std::move(eventSymbol), trade) {}

    /**
     * Creates new trade with the specified event symbol
     *
     * @param eventSymbol The event symbol
     */
    explicit Trade(std::string eventSymbol) : TradeBase(std::move(eventSymbol)) {}

    /// Returns a string representation of the trade
    std::string toString() const override { return std::string("Trade{") + baseFieldsToString() + "}"; }
};

/**
 * TradeETH event is a snapshot of the price and size of the last trade during
 * extended trading hours and the extended trading hours day volume and day turnover.
 * This event is defined only for symbols (typically stocks and ETFs) with a designated
 * <b>extended trading hours</b>  (ETH, pre market and post market trading sessions).
 * It represents the most recent information that is available about
 * ETH last trade on the market at any given moment of time.
 */
struct TradeETH final : public TradeBase {
    /// The alias to a type of shared pointer to the TradeETH object
    using Ptr = std::shared_ptr<TradeETH>;

    /**
     * Creates new trade with the specified event symbol and dxFeed C-API dxf_trade_t
     *
     * @param eventSymbol The event symbol
     * @param trade The dxFeed C-API dxf_trade_t object
     */
    TradeETH(std::string eventSymbol, const dxf_trade_eth_t &trade) : TradeBase(std::move(eventSymbol), trade) {}

    /**
     * Creates new trade with the specified event symbol
     *
     * @param eventSymbol The event symbol
     */
    explicit TradeETH(std::string eventSymbol) : TradeBase(std::move(eventSymbol)) {}

    /// Returns a string representation of the trade
    std::string toString() const override { return std::string("TradeETH{") + baseFieldsToString() + "}"; }
};

} // namespace dxfcpp