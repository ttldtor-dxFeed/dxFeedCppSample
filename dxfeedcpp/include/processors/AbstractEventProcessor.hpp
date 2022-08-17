#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include "events/Event.hpp"

#include <string>

namespace dxfcpp {

/**
 * The abstract class that represents event processing entity
 */
struct AbstractEventProcessor : dxfcpp::SharedEntity {
    /// The alias to a type of shared pointer to the processor object
    using Ptr = std::shared_ptr<AbstractEventProcessor>;

    /**
     * Processes the event
     * @param event The dxFeed C++-API event pointer
     */
    virtual void process(dxfcpp::Event::Ptr event) = 0;

    /// Returns a string representation of the entity
    std::string toString() const override {
        return std::string("AbstractEventProcessor{") + string::toHex((uint64_t)this) + "}";
    }
};

} // namespace dxfcpp