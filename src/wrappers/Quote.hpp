#pragma once

#include "EventData.h"
#include <string>
#include <utility>

#include "DXFCppConfig.hpp"

#include "Event.hpp"
#include "OrderScope.hpp"
#include "converters/DateTimeConverter.hpp"
#include "converters/StringConverter.hpp"

namespace dxfcpp {

/**
 * @brief Quote.
 *
 * @details Quote event is a snapshot of the best bid and ask prices, and other fields that change with each quote. It
 * represents the most recent information that is available about the best quote on the market at any given moment of
 * time.
 */
struct Quote : virtual public MarketEvent, virtual public Lasting {
    using Ptr = std::shared_ptr<Quote>;

  private:
    /// Time of the last bid or ask change
    std::uint64_t time_{};
    /// Sequence number of this quote to distinguish quotes that have the same #time.
    std::int32_t sequence_{};
    /// Microseconds and nanoseconds part of time of the last bid or ask change
    std::int32_t timeNanoPart_{};
    /// Time of the last bid change
    std::uint64_t bidTime_{};
    /// Bid exchange code
    char bidExchangeCode_{};
    /// Bid price
    double bidPrice_{std::numeric_limits<double>::quiet_NaN()};
    /// Bid size
    double bidSize_{std::numeric_limits<double>::quiet_NaN()};
    /// Time of the last ask change
    std::uint64_t askTime_{};
    /// Ask exchange code
    char askExchangeCode_{};
    /// Ask price
    double askPrice_{std::numeric_limits<double>::quiet_NaN()};
    /// Ask size
    double askSize_{std::numeric_limits<double>::quiet_NaN()};
    /**
     * Scope of this quote.
     *
     * Possible values: #OrderScope::COMPOSITE(Quote events) , #OrderScope::REGIONAL (Quote& events)
     */
    OrderScope scope_;

  public:
    Quote() : MarketEvent(), scope_(OrderScope::UNKNOWN) {}

    Quote(const Quote &other)
        : MarketEvent(other), time_{other.time_}, sequence_{other.sequence_}, timeNanoPart_{other.timeNanoPart_},
          bidTime_{other.bidTime_}, bidExchangeCode_{other.bidExchangeCode_}, bidPrice_{other.bidPrice_},
          bidSize_{other.bidSize_}, askTime_{other.askTime_}, askExchangeCode_{other.askExchangeCode_},
          askPrice_{other.askPrice_}, askSize_{other.askSize_}, scope_{other.scope_} {}

    Quote &operator=(const Quote &other) {
        setEventSymbol(other.getEventSymbol());
        setEventTime(other.getEventTime());
        time_ = other.time_;
        sequence_ = other.sequence_;
        timeNanoPart_ = other.timeNanoPart_;
        bidTime_ = other.bidTime_;
        bidExchangeCode_ = other.bidExchangeCode_;
        bidPrice_ = other.bidPrice_;
        bidSize_ = other.bidSize_;
        askTime_ = other.askTime_;
        askExchangeCode_ = other.askExchangeCode_;
        askPrice_ = other.askPrice_;
        askSize_ = other.askSize_;
        scope_ = other.scope_;

        return *this;
    }

    Quote(Quote &&other) noexcept
        : MarketEvent(std::move(other)), time_{other.time_}, sequence_{other.sequence_},
          timeNanoPart_{other.timeNanoPart_}, bidTime_{other.bidTime_}, bidExchangeCode_{other.bidExchangeCode_},
          bidPrice_{other.bidPrice_}, bidSize_{other.bidSize_}, askTime_{other.askTime_},
          askExchangeCode_{other.askExchangeCode_}, askPrice_{other.askPrice_}, askSize_{other.askSize_},
          scope_{std::move(other.scope_)} {}

    Quote &operator=(Quote &&other) noexcept {
        MarketEvent::operator=(std::move(other));
        time_ = other.time_;
        sequence_ = other.sequence_;
        timeNanoPart_ = other.timeNanoPart_;
        bidTime_ = other.bidTime_;
        bidExchangeCode_ = other.bidExchangeCode_;
        bidPrice_ = other.bidPrice_;
        bidSize_ = other.bidSize_;
        askTime_ = other.askTime_;
        askExchangeCode_ = other.askExchangeCode_;
        askPrice_ = other.askPrice_;
        askSize_ = other.askSize_;
        scope_ = std::move(other.scope_);

        return *this;
    }

    explicit Quote(std::string eventSymbol) : MarketEvent(std::move(eventSymbol)), scope_(OrderScope::UNKNOWN) {}
    void setEventSymbol(const std::string &eventSymbol) override { MarketEvent::setEventSymbol(eventSymbol); }
    void setEventTime(std::uint64_t eventTime) override { MarketEvent::setEventTime(eventTime); }
    Quote(std::string eventSymbol, dxf_quote_t const &quote)
        : MarketEvent(std::move(eventSymbol)), time_{static_cast<std::uint64_t>(quote.time)}, sequence_{quote.sequence},
          timeNanoPart_{quote.time_nanos}, bidTime_{static_cast<std::uint64_t>(quote.bid_time)},
          bidExchangeCode_{StringConverter::wCharToUtf8(quote.bid_exchange_code)}, bidPrice_{quote.bid_price},
          bidSize_{quote.bid_size}, askTime_{static_cast<std::uint64_t>(quote.ask_time)},
          askExchangeCode_{StringConverter::wCharToUtf8(quote.ask_exchange_code)}, askPrice_{quote.ask_price},
          askSize_{quote.ask_size}, scope_{OrderScope::get(quote.scope)} {}
    const std::string &getEventSymbol() const override { return MarketEvent::getEventSymbol(); }
    std::uint64_t getEventTime() const override { return MarketEvent::getEventTime(); }
    std::uint64_t getTime() const { return time_; }
    std::int32_t getSequence() const { return sequence_; }
    std::int32_t getTimeNanoPart() const { return timeNanoPart_; }
    std::uint64_t getBidTime() const { return bidTime_; }
    char getBidExchangeCode() const { return bidExchangeCode_; }
    double getBidPrice() const { return bidPrice_; }
    double getBidSize() const { return bidSize_; }
    std::uint64_t getAskTime() const { return askTime_; }
    char getAskExchangeCode() const { return askExchangeCode_; }
    double getAskPrice() const { return askPrice_; }
    double getAskSize() const { return askSize_; }
    const OrderScope &getScope() const { return scope_; }

    std::string toString() const override {
        return std::string("Quote") + "{" + getEventSymbol() + ", eventTime=" + DateTimeConverter::toISO(getEventTime()) +
            ", timeNanoPart=" + std::to_string(timeNanoPart_) + ", sequence=" + std::to_string(sequence_) +
            ", bidTime=" + DateTimeConverter::toISO(bidTime_) + ", bidExchange=" + bidExchangeCode_ +
            ", bidPrice=" + std::to_string(bidPrice_) + ", bidSize=" + std::to_string(bidSize_) +
            ", askTime=" + DateTimeConverter::toISO(askTime_) + ", askExchange=" + askExchangeCode_ +
            ", askPrice=" + std::to_string(askPrice_) + ", askSize=" + std::to_string(askSize_) +
            ", scope=" + scope_.toString() + "}";
    }

    template <class Ostream> friend Ostream &&operator<<(Ostream &&os, const Quote &value) {
        return std::forward<Ostream>(os) << value.toString();
    }

    ~Quote() override = default;
};

} // namespace dxfcpp