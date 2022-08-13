#pragma once

#ifndef DXFEED_HPP_INCLUDED
#error Please include only the DXFeed.hpp header
#endif

#include "common/DXFCppConfig.hpp"

#include "date/date.h"

#include <chrono>
#include <sstream>
#include <string>

namespace dxfcpp {

namespace DateTimeConverter {
inline long long parseISO(const std::string &s) {
    std::istringstream in{s};
    date::sys_time<std::chrono::milliseconds> tp;
    in >> date::parse("%FT%TZ", tp);

    if (in.fail()) {
        in.clear();
        in.exceptions(std::ios::failbit);
        in.str(s);
        in >> date::parse("%FT%T%Ez", tp);
    }

    return tp.time_since_epoch().count();
}

inline std::string toISO(long long timestamp) {
    return date::format("%FT%TZ", date::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{timestamp}});
}

inline std::string toISO(std::uint64_t timestamp) {
    return date::format("%FT%TZ", date::sys_time<std::chrono::milliseconds>{std::chrono::milliseconds{timestamp}});
}

} // namespace DateTimeConverter

} // namespace dxfcpp