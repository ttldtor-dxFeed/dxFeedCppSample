#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include "AbstractEventProcessor.hpp"

namespace dxfcpp {

/**
 * The abstract class that represents event processing entity with checking events by type
 *
 * @tparam E The dxFeed C++-API event type
 */
template <typename E> struct AbstractEventCheckingProcessor : AbstractEventProcessor {
    /**
     * Processes the event of type std::shared_ptr<E>
     *
     * @param e The event
     */
    virtual void process(typename E::Ptr e) = 0;

    /**
     * Tries to convert event pointer to pointer type to E and process it.
     *
     * @param e The event
     */
    void process(dxfcpp::Event::Ptr e) override {
        auto event = e->sharedAs<E>();

        if (!event)
            return;

        process(event);
    }
};

} // namespace dxfcpp