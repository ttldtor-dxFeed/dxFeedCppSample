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

template <> struct hash<dxf_price_type_t> {
    std::size_t operator()(dxf_price_type_t value) const noexcept { return static_cast<std::size_t>(value); }
};

} // namespace std

namespace dxfcpp {

/**
 * Wrapper over the dxf_price_type_t enum
 *
 * Type of the price value.
 */
class PriceType {
    dxf_price_type_t priceType_;

    std::string name_;

    PriceType(dxf_price_type_t priceType, std::string name) : priceType_{priceType}, name_{std::move(name)} {}

  public:
    /**
     * Regular price.
     */
    static const PriceType REGULAR;

    /**
     * Indicative price (derived via math formula).
     */
    static const PriceType INDICATIVE;

    /**
     * Preliminary price (preliminary settlement price), usually posted prior to #FINAL price.
     */
    static const PriceType PRELIMINARY;

    /**
     * Final price (final settlement price).
     */
    static const PriceType FINAL;

    /**
     * Unknown price type
     */
    static const PriceType UNKNOWN;

    const static std::unordered_map<dxf_price_type_t, PriceType> ALL;

    static PriceType get(dxf_price_type_t priceType) {
        auto found = ALL.find(priceType);

        if (found != ALL.end()) {
            return found->second;
        }

        return UNKNOWN;
    }

    dxf_price_type_t getPriceType() const { return priceType_; }

    const std::string &getName() const { return name_; }

    std::string toString() const { return name_; }

    template <class Ostream> friend Ostream &&operator<<(Ostream &&os, const PriceType &value) {
        return std::forward<Ostream>(os) << value.toString();
    }
};

const PriceType PriceType::REGULAR{dxf_pt_regular, "Regular"};
const PriceType PriceType::INDICATIVE{dxf_pt_indicative, "Indicative"};
const PriceType PriceType::PRELIMINARY{dxf_pt_preliminary, "Preliminary"};
const PriceType PriceType::FINAL{dxf_pt_final, "Final"};
const PriceType PriceType::UNKNOWN{static_cast<dxf_price_type_t>(256), "Unknown"};

const std::unordered_map<dxf_price_type_t, PriceType> PriceType::ALL{{REGULAR.getPriceType(), REGULAR},
                                                                     {INDICATIVE.getPriceType(), INDICATIVE},
                                                                     {PRELIMINARY.getPriceType(), PRELIMINARY},
                                                                     {FINAL.getPriceType(), FINAL}};

} // namespace dxfcpp