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

struct Configuration final : virtual public Event, virtual public Lasting {
  private:
    std::string eventSymbol_{};

  public:
    std::string toString() const override { return std::string(); }
    const std::string &getEventSymbol() const override { return eventSymbol_; }
    void setEventSymbol(const std::string &string) override {}
    uint64_t getEventTime() const override { return 0; }
    void setEventTime(std::uint64_t uint64) override {}
};

} // namespace dxfcpp