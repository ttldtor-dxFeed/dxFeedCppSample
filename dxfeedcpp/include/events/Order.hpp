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

struct OrderBase : virtual public MarketEvent, virtual public Indexed {
  private:
    std::string eventSymbol_{};
    /// Transactional event flags
    EventFlagsMask eventFlags_{};

  public:
    // TODO: OrderSource
    const IndexedEventSource &getSource() const override { return IndexedEventSource::DEFAULT; }
    const EventFlagsMask &getEventFlags() const override { return eventFlags_; }
    void setEventFlags(const EventFlagsMask &mask) override {}
    uint64_t getIndex() const override { return 0; }
    void setIndex(std::uint64_t uint64) override {}
};

struct Order final : virtual public OrderBase {
    std::string toString() const override { return std::string(); }
};

struct SpreadOrder final : virtual public OrderBase {
    std::string toString() const override { return std::string(); }
};

} // namespace dxfcpp