#pragma once

#ifndef DXFEED_HPP_INCLUDED
#error Please include only the DXFeed.hpp header
#endif

#include "AbstractEventProcessor.hpp"

namespace dxfcpp {

template <typename E> struct AbstractEventCheckingProcessor : AbstractEventProcessor {
    virtual void process(typename E::Ptr e) = 0;

    void process(dxfcpp::Event::Ptr e) override {
        if (!e)
            return;
        if (!e->is<E>())
            return;

        auto event = e->sharedAs<E>();

        if (!event)
            return;

        process(event);
    }
};

} // namespace dxfcpp