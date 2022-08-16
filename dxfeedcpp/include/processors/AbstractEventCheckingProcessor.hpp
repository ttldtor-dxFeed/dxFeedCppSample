#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include "AbstractEventProcessor.hpp"

namespace dxfcpp {

/**
 *
 * @tparam E
 */
template <typename E> struct AbstractEventCheckingProcessor : AbstractEventProcessor {
    /**
     *
     * @param e
     */
    virtual void process(typename E::Ptr e) = 0;

    /**
     *
     * @param e
     */
    void process(dxfcpp::Event::Ptr e) override {
        auto event = e->sharedAs<E>();

        if (!event)
            return;

        process(event);
    }
};

} // namespace dxfcpp