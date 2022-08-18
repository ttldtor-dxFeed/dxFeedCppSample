#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include "AbstractEventProcessor.hpp"

#include <vector>

namespace dxfcpp {

/// The thread-safe event processor that applies an event to the vector of childs of AbstractEventProcessor
class CompositeProcessor : virtual public AbstractEventProcessor {
    const std::vector<AbstractEventProcessor::Ptr> processors_{};

  public:
    /**
     * Creates the new composite processor
     *
     * @tparam ProcessorIt The type of iterator of the processors container
     * @param begin The first iterator of the processors container
     * @param end The last iterator of the processors container
     */
    template <typename ProcessorIt>
    explicit CompositeProcessor(ProcessorIt begin, ProcessorIt end) noexcept : processors_{begin, end} {}

    /**
     * Creates the new composite processor
     *
     * @param processors The initializer list of processors
     */
    CompositeProcessor(std::initializer_list<AbstractEventProcessor::Ptr> processors) noexcept
        : CompositeProcessor(processors.begin(), processors.end()) {}

    /**
     * Applies the event to processors
     *
     * @param event The dxFeed C++-API event pointer
     */
    void process(dxfcpp::Event::Ptr event) override {
        for (const auto &p : processors_) {
            p->process(event);
        }
    }
};

} // namespace dxfcpp