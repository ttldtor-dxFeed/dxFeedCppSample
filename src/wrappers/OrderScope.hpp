#pragma once

#include <string>
#include <unordered_map>

namespace dxfcs {

/**
 * Wrapper over the dxf_order_scope_t enum
 */
class OrderScope {
    unsigned scope_;
    std::string string_;

  public:
    static const OrderScope COMPOSITE;
    static const OrderScope REGIONAL;
    static const OrderScope AGGREGATE;
    static const OrderScope ORDER;

    OrderScope(unsigned scope, std::string string) : scope_{scope}, string_{std::move(string)} {}

    static OrderScope get(unsigned scope) {
        static const std::unordered_map<unsigned, OrderScope> data{{COMPOSITE.getScope(), COMPOSITE},
                                                                   {REGIONAL.getScope(), REGIONAL},
                                                                   {AGGREGATE.getScope(), AGGREGATE},
                                                                   {ORDER.getScope(), ORDER}};

        return data.at(scope);
    }

    unsigned getScope() const { return scope_; }

    const std::string &toString() const { return string_; }
};

const OrderScope OrderScope::COMPOSITE{0u, "Composite"};
const OrderScope OrderScope::REGIONAL{1u, "Regional"};
const OrderScope OrderScope::AGGREGATE{2u, "Aggregate"};
const OrderScope OrderScope::ORDER{3u, "Order"};

} // namespace dxfcs