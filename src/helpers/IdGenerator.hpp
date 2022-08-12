#pragma once

#include <cstdint>

namespace dxfcpp {

struct IdGenerator {
    static std::uint64_t getId() {
        static std::uint64_t currentId{};

        return currentId++;
    }
};

} // namespace dxfcpp