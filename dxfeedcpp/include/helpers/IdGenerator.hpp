#pragma once

#ifndef DXFEED_HPP_INCLUDED
#error Please include only the DXFeed.hpp header
#endif

#include <cstdint>

namespace dxfcpp {

struct IdGenerator {
    static std::uint64_t getId() {
        static std::uint64_t currentId{};

        return currentId++;
    }
};

} // namespace dxfcpp