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

struct TradeBase : virtual public MarketEvent, virtual public Lasting {};

struct Trade final : virtual public TradeBase {
    Trade(std::string eventSymbol, const dxf_trade_t &trade) {}
    std::string toString() const override { return std::string(); }
};

struct TradeETH final : virtual public TradeBase {
    TradeETH(std::string eventSymbol, const dxf_trade_eth_t &trade) {}
    std::string toString() const override { return std::string(); }
};

} // namespace dxfcpp