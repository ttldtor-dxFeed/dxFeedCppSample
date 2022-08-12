#pragma once

#include "DXFCppConfig.hpp"

#include <numeric>
#include <sstream>
#include <unordered_set>
#include <utility>

namespace dxfcpp {

class EventFlagsMask;

/**
 * Wrapper over the dxf_event_flag enum
 */
class EventFlag {
    unsigned flag_;
    std::string name_;

    EventFlag(unsigned flag, std::string name) : flag_{flag}, name_{std::move(name)} {}

  public:
    static const EventFlag TX_PENDING;
    static const EventFlag REMOVE_EVENT;
    static const EventFlag SNAPSHOT_BEGIN;
    static const EventFlag SNAPSHOT_END;
    static const EventFlag SNAPSHOT_SNIP;
    static const EventFlag SNAPSHOT_MODE;
    static const EventFlag REMOVE_SYMBOL;

    explicit EventFlag() : flag_{unsigned(-1)}, name_{"INVALID"} {}

    unsigned getFlag() const { return flag_; }

    bool in(unsigned eventFlagsMask) const { return (eventFlagsMask & flag_) != 0; }

    template <typename EventFlagsMask> bool in(const EventFlagsMask &eventFlagsMask) const {
        return in(eventFlagsMask.getMask());
    }

    const std::string &getName() const { return name_; }

    std::string toString() const { return name_; }
};

const EventFlag EventFlag::TX_PENDING{0x01u, "TX_PENDING"};
const EventFlag EventFlag::REMOVE_EVENT{0x02u, "REMOVE_EVENT"};
const EventFlag EventFlag::SNAPSHOT_BEGIN{0x04u, "SNAPSHOT_BEGIN"};
const EventFlag EventFlag::SNAPSHOT_END{0x08u, "SNAPSHOT_END"};
const EventFlag EventFlag::SNAPSHOT_SNIP{0x10u, "SNAPSHOT_SNIP"};
// 0x20 is reserved. This flag will fit into 1-byte on the wire in QTP protocol
const EventFlag EventFlag::SNAPSHOT_MODE{0x40u, "SNAPSHOT_MODE"};
const EventFlag EventFlag::REMOVE_SYMBOL{0x80u, "REMOVE_SYMBOL"};

} // namespace dxfcpp

namespace std {
template <> struct hash<dxfcpp::EventFlag> {
    std::size_t operator()(const dxfcpp::EventFlag &eventFlag) const noexcept { return eventFlag.getFlag(); }
};
} // namespace std

namespace dxfcpp {

class EventFlagsMask {
    unsigned mask_;

  public:
    explicit EventFlagsMask() : mask_{0u} {}
    explicit EventFlagsMask(unsigned mask) : mask_{mask} {}
    explicit EventFlagsMask(dxf_event_flag_t mask) : mask_{static_cast<unsigned>(mask)} {}

    template <typename EventFlagIt> EventFlagsMask(EventFlagIt begin, EventFlagIt end) {
        mask_ =
            std::accumulate(begin, end, 0u, [](unsigned mask, const EventFlag &flag) { return mask | flag.getFlag(); });
    }

    explicit EventFlagsMask(std::initializer_list<EventFlag> eventTypes)
        : EventFlagsMask(eventTypes.begin(), eventTypes.end()) {}

    DXFCPP_CONSTEXPR unsigned getMask() const { return mask_; }

    friend EventFlagsMask operator|(const EventFlagsMask &eventTypesMask, const EventFlag &eventType) {
        return EventFlagsMask{eventTypesMask.mask_ | eventType.getFlag()};
    }

    friend EventFlagsMask operator|(const EventFlag &eventType1, const EventFlag &eventType2) {
        return EventFlagsMask{eventType1.getFlag() | eventType2.getFlag()};
    }

    std::string toString() const {
        bool addOrSign = false;
        std::ostringstream result{};

        for (const auto &flag :
             {EventFlag::TX_PENDING, EventFlag::REMOVE_EVENT, EventFlag::SNAPSHOT_BEGIN, EventFlag::SNAPSHOT_END,
              EventFlag::SNAPSHOT_SNIP, EventFlag::SNAPSHOT_MODE, EventFlag::REMOVE_SYMBOL}) {
            if (flag.in(mask_)) {
                if (addOrSign) {
                    result << "|";
                } else {
                    addOrSign = true;
                }

                result << flag.toString();
            }
        }

        result << "(0x" << std::hex << mask_ << ")";

        return result.str();
    }
};

} // namespace dxfcpp