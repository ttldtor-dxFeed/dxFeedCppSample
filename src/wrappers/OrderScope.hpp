#pragma once

#include <EventData.h>
#include <string>
#include <unordered_map>

namespace std {

template<>
struct hash<dxf_order_scope_t> {
    std::size_t operator()(dxf_order_scope_t scope) const noexcept {
        return static_cast<std::size_t>(scope);
    }
};

}

namespace dxfcs {

/**
 * Wrapper over the dxf_order_scope_t enum
 */
class OrderScope {
    dxf_order_scope_t scope_;
    std::string string_;

    OrderScope(dxf_order_scope_t scope, std::string string) : scope_{scope}, string_{std::move(string)} {}

  public:
    static const OrderScope COMPOSITE;
    static const OrderScope REGIONAL;
    static const OrderScope AGGREGATE;
    static const OrderScope ORDER;

    const static std::unordered_map<dxf_order_scope_t, OrderScope> ALL;

    static OrderScope get(dxf_order_scope_t scope) { return ALL.at(scope); }

    dxf_order_scope_t getScope() const { return scope_; }

    const std::string &toString() const { return string_; }

    template <class Ostream> friend Ostream &&operator<<(Ostream &&os, const OrderScope &value) {
        return std::forward<Ostream>(os) << value.toString();
    }
};

const OrderScope OrderScope::COMPOSITE{dxf_osc_composite, "Composite"};
const OrderScope OrderScope::REGIONAL{dxf_osc_regional, "Regional"};
const OrderScope OrderScope::AGGREGATE{dxf_osc_aggregate, "Aggregate"};
const OrderScope OrderScope::ORDER{dxf_osc_order, "Order"};

const std::unordered_map<dxf_order_scope_t, OrderScope> OrderScope::ALL{{COMPOSITE.getScope(), COMPOSITE},
                                                                               {REGIONAL.getScope(), REGIONAL},
                                                                               {AGGREGATE.getScope(), AGGREGATE},
                                                                               {ORDER.getScope(), ORDER}};

} // namespace dxfcs