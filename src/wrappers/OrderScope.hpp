#pragma once

#include <EventData.h>
#include <string>
#include <unordered_map>

namespace dxfcs {

/**
 * Wrapper over the dxf_order_scope_t enum
 */
class OrderScope {
    dxf_order_scope_t scope_;
    std::string string_;

  public:
    static const OrderScope COMPOSITE;
    static const OrderScope REGIONAL;
    static const OrderScope AGGREGATE;
    static const OrderScope ORDER;

    OrderScope(dxf_order_scope_t scope, std::string string) : scope_{scope}, string_{std::move(string)} {}

    static OrderScope get(dxf_order_scope_t scope) {
        static const std::unordered_map<dxf_order_scope_t, OrderScope> data{{COMPOSITE.getScope(), COMPOSITE},
                                                                            {REGIONAL.getScope(), REGIONAL},
                                                                            {AGGREGATE.getScope(), AGGREGATE},
                                                                            {ORDER.getScope(), ORDER}};

        return data.at(scope);
    }

    dxf_order_scope_t getScope() const { return scope_; }

    const std::string &toString() const { return string_; }
};

const OrderScope OrderScope::COMPOSITE{dxf_osc_composite, "Composite"};
const OrderScope OrderScope::REGIONAL{dxf_osc_regional, "Regional"};
const OrderScope OrderScope::AGGREGATE{dxf_osc_aggregate, "Aggregate"};
const OrderScope OrderScope::ORDER{dxf_osc_order, "Order"};

} // namespace dxfcs