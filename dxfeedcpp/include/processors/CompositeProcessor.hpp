#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include "AbstractEventProcessor.hpp"

#include <vector>

namespace dxfcpp {

///
class CompositeProcessor : virtual public AbstractEventProcessor {
    std::vector<AbstractEventProcessor::Ptr> processors_{};

  public:
    /**
     *
     * @tparam ProcessorIt
     * @param begin
     * @param end
     */
    template <typename ProcessorIt> explicit CompositeProcessor(ProcessorIt begin, ProcessorIt end) noexcept {
        for (auto it = begin; it != end; it++) {
            processors_.push_back(*it);
        }
    }

    /**
     *
     * @param processors
     */
    CompositeProcessor(std::initializer_list<AbstractEventProcessor::Ptr> processors) noexcept
        : CompositeProcessor(processors.begin(), processors.end()) {}

    void process(dxfcpp::Event::Ptr event) override {
        for (auto p : processors_) {
            p->process(event);
        }
    }
};

} // namespace dxfcpp