#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#include "common/DXFCppConfig.hpp"

#include <cstdint>
#include <sstream>

namespace dxfcpp {

namespace math {

/**
 * Returns quotient according to number theory - i.e. when remainder is zero or positive.
 *
 * @param a dividend
 * @param b divisor
 * @return quotient according to number theory
 */
static inline std::int32_t div(std::int32_t a, std::int32_t b) {
    return a >= 0 ? a / b : b >= 0 ? (a + 1) / b - 1 : (a + 1) / b + 1;
}

} // namespace math

namespace day_util {

/**
 * Returns yyyymmdd integer in Gregorian calendar for a specified day identifier.
 * The day identifier is defined as the number of days since Unix epoch of January 1, 1970.
 * The result is equal to <pre>yearSign * (abs(year) * 10000 + month * 100 + day)</pre>, where year,
 * month, and day are in Gregorian calendar, month is between 1 and 12 inclusive, and day is counted from 1.
 *
 * @param dayId The number of days since Unix epoch of January 1, 1970.
 * @return yyyymmdd integer
 */
static inline std::int32_t getYearMonthDayByDayId(std::int32_t dayId) {
    std::int32_t j = dayId + 2472632; // this shifts the epoch back to astronomical year -4800
    std::int32_t g = math::div(j, 146097);
    std::int32_t dg = j - g * 146097;
    std::int32_t c = (dg / 36524 + 1) * 3 / 4;
    std::int32_t dc = dg - c * 36524;
    std::int32_t b = dc / 1461;
    std::int32_t db = dc - b * 1461;
    std::int32_t a = (db / 365 + 1) * 3 / 4;
    std::int32_t da = db - a * 365;
    std::int32_t y = g * 400 + c * 100 + b * 4 +
        a; // this is the integer number of full years elapsed since March 1, 4801 BC at 00:00 UTC
    std::int32_t m = (da * 5 + 308) / 153 -
        2; // this is the integer number of full months elapsed since the last March 1 at 00:00 UTC
    std::int32_t d =
        da - (m + 4) * 153 / 5 + 122; // this is the number of days elapsed since day 1 of the month at 00:00 UTC
    std::int32_t yyyy = y - 4800 + (m + 2) / 12;
    std::int32_t mm = (m + 2) % 12 + 1;
    std::int32_t dd = d + 1;
    std::int32_t yyyymmdd = std::abs(yyyy) * 10000 + mm * 100 + dd;
    return yyyy >= 0 ? yyyymmdd : -yyyymmdd;
}
} // namespace day_util

namespace string {

static inline std::string toHex(std::uint64_t v) {
    std::ostringstream oss{};

    oss << "0x" << std::hex << v;

    return oss.str();
}

} // namespace string

} // namespace dxfcpp