#pragma once

#include "DXFCppConfig.hpp"

#include <sstream>

namespace dxfcpp {

/**
 * Wrapper over the dxf_event_flag enum
 */
class EventFlags {
    unsigned flags_;
    std::string string_;

    explicit EventFlags(unsigned flag, std::string string) : flags_{flag}, string_{std::move(string)} {}

  public:
    static const EventFlags TX_PENDING;
    static const EventFlags REMOVE_EVENT;
    static const EventFlags SNAPSHOT_BEGIN;
    static const EventFlags SNAPSHOT_END;
    static const EventFlags SNAPSHOT_SNIP;
    static const EventFlags SNAPSHOT_MODE;
    static const EventFlags REMOVE_SYMBOL;

    static std::vector<EventFlags> get(unsigned flags) {
        std::vector<EventFlags> result{};

        for (const auto &flag :
             {TX_PENDING, REMOVE_EVENT, SNAPSHOT_BEGIN, SNAPSHOT_END, SNAPSHOT_SNIP, SNAPSHOT_MODE, REMOVE_SYMBOL}) {

            if (flag.in(flags)) {
                result.emplace_back(flag);
            }
        }

        return result;
    }

    unsigned getFlag() const { return flags_; }

    bool in(unsigned eventFlags) const { return (eventFlags & flags_) != 0; }

    bool in(int eventFlags) const { return in(static_cast<unsigned>(eventFlags)); }

    unsigned set(unsigned eventFlags) const { return eventFlags | flags_; }

    int set(int eventFlags) const { return static_cast<int>(set(static_cast<unsigned>(eventFlags))); }

    unsigned clear(unsigned eventFlags) const { return eventFlags & ~flags_; }

    int clear(int eventFlags) const { return static_cast<int>(clear(static_cast<unsigned>(eventFlags))); }

    const std::string& getString() const {
        return string_;
    }

    static std::string toString(unsigned eventFlags) {
        bool addOrSign = false;
        std::ostringstream result{};

        for (const auto &flag :
             {TX_PENDING, REMOVE_EVENT, SNAPSHOT_BEGIN, SNAPSHOT_END, SNAPSHOT_SNIP, SNAPSHOT_MODE, REMOVE_SYMBOL}) {
            if (flag.in(eventFlags)) {
                if (addOrSign) {
                    result << "|";
                } else {
                    addOrSign = true;
                }

                result << flag.getString();
            }
        }

        result << "(0x" << std::hex << eventFlags << ")";

        return result.str();
    }
};

const EventFlags EventFlags::TX_PENDING{0x01u, "TX_PENDING"};
const EventFlags EventFlags::REMOVE_EVENT{0x02u, "REMOVE_EVENT"};
const EventFlags EventFlags::SNAPSHOT_BEGIN{0x04u, "SNAPSHOT_BEGIN"};
const EventFlags EventFlags::SNAPSHOT_END{0x08u, "SNAPSHOT_END"};
const EventFlags EventFlags::SNAPSHOT_SNIP{0x10u, "SNAPSHOT_SNIP"};
// 0x20 is reserved. This flag will fit into 1-byte on the wire in QTP protocol
const EventFlags EventFlags::SNAPSHOT_MODE{0x40u, "SNAPSHOT_MODE"};
const EventFlags EventFlags::REMOVE_SYMBOL{0x80u, "REMOVE_SYMBOL"};

} // namespace dxfcpp