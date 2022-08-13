#pragma once

#ifndef DXFEED_HPP_INCLUDED
#error Please include only the DXFeed.hpp header
#endif

#include "events/Event.hpp"

#include <string>

namespace dxfcpp {

struct AbstractEventProcessor : dxfcpp::SharedEntity {
    using Ptr = std::shared_ptr<AbstractEventProcessor>;
    virtual void process(dxfcpp::Event::Ptr event) = 0;
    std::string toString() const override { return ""; }
};

} // namespace dxfcpp