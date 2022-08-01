#pragma once

namespace dxfcs {

/**
 * Wrapper over the dxf_event_flag enum
 */
class EventFlag {
    unsigned flag_;

  public:
    static const EventFlag TX_PENDING;
    static const EventFlag REMOVE_EVENT;
    static const EventFlag SNAPSHOT_BEGIN;
    static const EventFlag SNAPSHOT_END;
    static const EventFlag SNAPSHOT_SNIP;
    static const EventFlag SNAPSHOT_MODE;
    static const EventFlag REMOVE_SYMBOL;

    explicit EventFlag(unsigned flag) : flag_{flag} {}
    explicit EventFlag(int flag) : EventFlag{static_cast<unsigned>(flag)} {}

    unsigned getFlag() const { return flag_; }

    bool in(unsigned eventFlags) const { return (eventFlags & flag_) != 0; }

    bool in(int eventFlags) const { return in(static_cast<unsigned>(eventFlags)); }

    unsigned set(unsigned eventFlags) const { return eventFlags | flag_; }

    int set(int eventFlags) const { return static_cast<int>(set(static_cast<unsigned>(eventFlags))); }

    unsigned clear(unsigned eventFlags) const { return eventFlags & ~flag_; }

    int clear(int eventFlags) const { return static_cast<int>(clear(static_cast<unsigned>(eventFlags))); }
};

const EventFlag EventFlag::TX_PENDING{0x01u};
const EventFlag EventFlag::REMOVE_EVENT{0x02u};
const EventFlag EventFlag::SNAPSHOT_BEGIN{0x04u};
const EventFlag EventFlag::SNAPSHOT_END{0x08u};
const EventFlag EventFlag::SNAPSHOT_SNIP{0x10u};
// 0x20 is reserved. This flag will fit into 1-byte on the wire in QTP protocol
const EventFlag EventFlag::SNAPSHOT_MODE{0x40u};
const EventFlag EventFlag::REMOVE_SYMBOL{0x80u};

} // namespace dxfcs