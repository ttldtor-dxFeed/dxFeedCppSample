#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

extern "C" {
#include <EventData.h>
}

#include <string>
#include <unordered_map>

#include "common/DXFCppConfig.hpp"

namespace std {

template <> struct hash<dxf_order_scope_t> {
    std::size_t operator()(dxf_order_scope_t scope) const noexcept { return static_cast<std::size_t>(scope); }
};

} // namespace std

namespace dxfcpp {

/**
 * Wrapper over the dxf_order_scope_t enum
 */
class OrderScope {
    dxf_order_scope_t scope_;
    std::string name_;

    OrderScope(dxf_order_scope_t scope, std::string name) : scope_{scope}, name_{std::move(name)} {}

  public:
    static const OrderScope COMPOSITE;
    static const OrderScope REGIONAL;
    static const OrderScope AGGREGATE;
    static const OrderScope ORDER;
    static const OrderScope UNKNOWN;

    const static std::unordered_map<dxf_order_scope_t, OrderScope> ALL;

    static OrderScope get(dxf_order_scope_t scope) {
        auto found = ALL.find(scope);

        if (found != ALL.end()) {
            return found->second;
        }

        return UNKNOWN;
    }

    dxf_order_scope_t getScope() const { return scope_; }

    const std::string &getName() const { return name_; }

    std::string toString() const { return name_; }

    template <class Ostream> friend Ostream &&operator<<(Ostream &&os, const OrderScope &value) {
        return std::forward<Ostream>(os) << value.toString();
    }
};

const OrderScope OrderScope::COMPOSITE{dxf_osc_composite, "Composite"};
const OrderScope OrderScope::REGIONAL{dxf_osc_regional, "Regional"};
const OrderScope OrderScope::AGGREGATE{dxf_osc_aggregate, "Aggregate"};
const OrderScope OrderScope::ORDER{dxf_osc_order, "Order"};
const OrderScope OrderScope::UNKNOWN{static_cast<dxf_order_scope_t>(256), "Unknown"};

const std::unordered_map<dxf_order_scope_t, OrderScope> OrderScope::ALL{{COMPOSITE.getScope(), COMPOSITE},
                                                                        {REGIONAL.getScope(), REGIONAL},
                                                                        {AGGREGATE.getScope(), AGGREGATE},
                                                                        {ORDER.getScope(), ORDER}};

} // namespace dxfcpp