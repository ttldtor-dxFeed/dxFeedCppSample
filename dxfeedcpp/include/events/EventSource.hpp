#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

namespace dxfcpp {

///
class IndexedEventSource {
    unsigned id_{};
    std::string name_{};

    IndexedEventSource(unsigned source_, std::string name) : id_{source_}, name_{std::move(name)} {}

  public:
    ///
    static const IndexedEventSource DEFAULT;

    ///
    unsigned int getSource() const { return id_; }

    ///
    const std::string &getName() const { return name_; }

    ///
    std::string toString() const { return name_; }
};

const IndexedEventSource IndexedEventSource::DEFAULT{0, "DEFAULT"};

} // namespace dxfcpp