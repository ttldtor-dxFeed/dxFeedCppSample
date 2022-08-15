#pragma once

#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

extern "C" {
#include <EventData.h>
}

#include <string>
#include <unordered_map>
#include <utility>

#include "common/DXFCppConfig.hpp"

namespace std {

template <> struct hash<dxf_direction_t> {
    std::size_t operator()(dxf_direction_t direction) const noexcept { return static_cast<std::size_t>(direction); }
};

}

namespace dxfcpp {

/**
 * Wrapper over the dxf_direction_t enum
 * Direction of the price movement. For example tick direction for last trade price.
 */
class Direction {
    dxf_direction_t direction_;
    std::string name_;

    Direction(dxf_direction_t direction, std::string name) : direction_{direction}, name_{std::move(name)} {}

  public:
    /**
     * Direction is undefined, unknown or inapplicable.
     * It includes cases with undefined price value or when direction computation was not performed.
     */
    static const Direction UNDEFINED;
    /**
     * Current price is lower than previous price.
     */
    static const Direction DOWN;
    /**
     * Current price is the same as previous price and is lower than the last known price of different value.
     */
    static const Direction ZERO_DOWN;
    /**
     * Current price is equal to the only known price value suitable for price direction computation.
     * Unlike <b>UNDEFINED</b> the <b>ZERO</b> direction implies that current price is defined and
     * direction computation was duly performed but has failed to detect any upward or downward price movement.
     * It is also reported for cases when price sequence was broken and direction computation was restarted anew.
     */
    static const Direction ZERO;
    /**
     * Current price is the same as previous price and is higher than the last known price of different value.
     */
    static const Direction ZERO_UP;
    /**
     * Current price is higher than previous price.
     */
    static const Direction UP;

    const static std::unordered_map<dxf_direction_t, Direction> ALL;

    static Direction get(dxf_direction_t direction) {
        auto found = ALL.find(direction);

        if (found != ALL.end()) {
            return found->second;
        }

        return UNDEFINED;
    }

    dxf_direction_t getDirection() const { return direction_; }

    const std::string &getName() const { return name_; }

    std::string toString() const { return name_; }

    template <class Ostream> friend Ostream &&operator<<(Ostream &&os, const Direction &value) {
        return std::forward<Ostream>(os) << value.toString();
    }

    bool operator == (const Direction& other) {
        return direction_ == other.direction_;
    }
};

const Direction Direction::UNDEFINED{dxf_dir_undefined, "UNDEFINED"};
const Direction Direction::DOWN{dxf_dir_down, "DOWN"};
const Direction Direction::ZERO_DOWN{dxf_dir_zero_down, "ZERO_DOWN"};
const Direction Direction::ZERO{dxf_dir_zero, "ZERO"};
const Direction Direction::ZERO_UP{dxf_dir_zero_up, "ZERO_UP"};
const Direction Direction::UP{dxf_dir_up, "UP"};

const std::unordered_map<dxf_direction_t, Direction> Direction::ALL{
    {UNDEFINED.getDirection(), UNDEFINED}, {DOWN.getDirection(), DOWN},       {ZERO_DOWN.getDirection(), ZERO_DOWN},
    {ZERO.getDirection(), ZERO},           {ZERO_UP.getDirection(), ZERO_UP}, {UP.getDirection(), UP},
};

} // namespace dxfcpp