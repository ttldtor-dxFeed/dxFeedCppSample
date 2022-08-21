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

namespace dxfcpp {

/**
 * @brief Candle
 *
 * @details Candle event with open, high, low, close prices and other information for a specific period.
 */
struct Candle final : public Event, public TimeSeries, public Lasting {
    /// The alias to a type of shared pointer to the Candle object
    using Ptr = std::shared_ptr<Candle>;

  private:
    std::string eventSymbol_{};
    /// Transactional event flags
    EventFlagsMask eventFlags_{};

    /**
     * Time when event was created or zero when time is not available. The time is nonzero for data events
     * that are read from historical tape files and from OnDemandService. Events that are coming from a network
     * connections do not have an embedded event time information and this field will contain zero for them, meaning
     * that event was received just now.
     */
    std::uint64_t eventTime_{};
    /// Unique per-symbol index of this candle
    std::uint64_t index_{};
    /// Timestamp of this candle in milliseconds
    std::uint64_t time_{};
    /// Sequence number of this candle; distinguishes candles with same #time_
    std::int32_t sequence_{};
    /// Total number of original trade (or quote) events in this candle
    std::uint64_t count_{};

    double open_{std::numeric_limits<double>::quiet_NaN()};
    double high_{std::numeric_limits<double>::quiet_NaN()};
    double low_{std::numeric_limits<double>::quiet_NaN()};
    double close_{std::numeric_limits<double>::quiet_NaN()};
    double volume_{std::numeric_limits<double>::quiet_NaN()};
    double vwap_{std::numeric_limits<double>::quiet_NaN()};
    double bidVolume_{std::numeric_limits<double>::quiet_NaN()};
    double askVolume_{std::numeric_limits<double>::quiet_NaN()};
    double impVolatility_{std::numeric_limits<double>::quiet_NaN()};
    double openInterest_{std::numeric_limits<double>::quiet_NaN()};

  public:
    Candle() = default;

    Candle(const Candle &other)
        : eventSymbol_{other.eventSymbol_}, eventFlags_{other.eventFlags_},
          eventTime_{other.eventTime_}, index_{other.index_}, time_{other.time_}, sequence_{other.sequence_},
          count_{other.count_}, open_{other.open_}, high_{other.high_}, low_{other.low_}, close_{other.close_},
          volume_{other.volume_}, vwap_{other.vwap_}, bidVolume_{other.bidVolume_}, askVolume_{other.askVolume_},
          impVolatility_{other.impVolatility_}, openInterest_{other.openInterest_} {}

    Candle &operator=(const Candle &other) = delete;

    Candle(Candle &&other) noexcept
        : eventSymbol_{std::move(other.eventSymbol_)}, eventFlags_{other.eventFlags_},
          eventTime_{other.eventTime_}, index_{other.index_}, time_{other.time_}, sequence_{other.sequence_},
          count_{other.count_}, open_{other.open_}, high_{other.high_}, low_{other.low_}, close_{other.close_},
          volume_{other.volume_}, vwap_{other.vwap_}, bidVolume_{other.bidVolume_}, askVolume_{other.askVolume_},
          impVolatility_{other.impVolatility_}, openInterest_{other.openInterest_} {}

    Candle &operator=(Candle &&other) noexcept = delete;

    explicit Candle(std::string eventSymbol) : eventSymbol_{std::move(eventSymbol)} {}

    Candle(std::string eventSymbol, const dxf_candle_t &candle)
        : eventSymbol_{std::move(eventSymbol)},
          eventFlags_(candle.event_flags), eventTime_{0}, index_{static_cast<std::uint64_t>(candle.index)},
          time_{static_cast<std::uint64_t>(candle.time)}, sequence_{candle.sequence},
          count_{static_cast<std::uint64_t>(candle.count)}, open_{candle.open}, high_{candle.high}, low_{candle.low},
          close_{candle.close}, volume_{candle.volume}, vwap_{candle.vwap}, bidVolume_{candle.bid_volume},
          askVolume_{candle.ask_volume}, impVolatility_{candle.ask_volume}, openInterest_{candle.open_interest} {}

    const std::string &getEventSymbol() const override { return eventSymbol_; }
    void setEventSymbol(const std::string &eventSymbol) override { eventSymbol_ = eventSymbol; }
    uint64_t getEventTime() const override { return eventTime_; }
    void setEventTime(std::uint64_t eventTime) override { eventTime_ = eventTime; }
    const IndexedEventSource &getSource() const override { return IndexedEventSource::DEFAULT; }
    const EventFlagsMask &getEventFlags() const override { return eventFlags_; }
    void setEventFlags(const EventFlagsMask &mask) override { eventFlags_ = mask; }
    uint64_t getIndex() const override { return index_; }
    void setIndex(std::uint64_t index) override { index_ = index; }
    uint64_t getTime() const override { return time_; }

    std::string toString() const override {
        return std::string("Candle{") + eventSymbol_ + ", index=" + std::to_string(index_) +
            ", eventTime=" + DateTimeConverter::toISO(eventTime_) + ", eventFlags=" + eventFlags_.toString() +
            ", time=" + DateTimeConverter::toISO(time_) + ", sequence=" + std::to_string(sequence_) +
            ", count=" + std::to_string(count_) + ", open=" + std::to_string(open_) +
            ", high=" + std::to_string(high_) + ", low=" + std::to_string(low_) + ", close=" + std::to_string(close_) +
            ", volume=" + std::to_string(volume_) + ", vwap=" + std::to_string(vwap_) +
            ", bidVolume=" + std::to_string(bidVolume_) + ", askVolume=" + std::to_string(askVolume_) +
            ", impVolatility=" + std::to_string(impVolatility_) + ", openInterest=" + std::to_string(openInterest_) +
            "}";
    }

    template <class Ostream> friend Ostream &&operator<<(Ostream &&os, const Candle &value) {
        return std::forward<Ostream>(os) << value.toString();
    }

    ~Candle() override = default;
};
} // namespace dxfcpp