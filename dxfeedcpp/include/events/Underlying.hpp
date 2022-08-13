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

struct Underlying final : virtual public MarketEvent, virtual public TimeSeries, virtual public Lasting {
  private:
    std::string eventSymbol_{};
    /// Transactional event flags
    EventFlagsMask eventFlags_{};

  public:
    std::string toString() const override { return std::string(); }
    const IndexedEventSource &getSource() const override { return IndexedEventSource::DEFAULT; }
    const EventFlagsMask &getEventFlags() const override { return eventFlags_; }
    void setEventFlags(const EventFlagsMask &mask) override {}
    uint64_t getIndex() const override { return 0; }
    void setIndex(std::uint64_t uint64) override {}
    uint64_t getTime() const override { return 0; }
};

} // namespace dxfcpp