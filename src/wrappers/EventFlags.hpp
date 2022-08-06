#pragma once

namespace dxfcpp {

/**
 * Wrapper over the dxf_event_flag enum
 */
class EventFlags {
    unsigned flags_;
    std::string string_;

  public:
    static const EventFlags TX_PENDING;
    static const EventFlags REMOVE_EVENT;
    static const EventFlags SNAPSHOT_BEGIN;
    static const EventFlags SNAPSHOT_END;
    static const EventFlags SNAPSHOT_SNIP;
    static const EventFlags SNAPSHOT_MODE;
    static const EventFlags REMOVE_SYMBOL;

    explicit EventFlags(unsigned flag) : flags_{flag} {}
    explicit EventFlags(int flag) : EventFlags{static_cast<unsigned>(flag)} {}

    unsigned getFlag() const { return flags_; }

    bool in(unsigned eventFlags) const { return (eventFlags & flags_) != 0; }

    bool in(int eventFlags) const { return in(static_cast<unsigned>(eventFlags)); }

    unsigned set(unsigned eventFlags) const { return eventFlags | flags_; }

    int set(int eventFlags) const { return static_cast<int>(set(static_cast<unsigned>(eventFlags))); }

    unsigned clear(unsigned eventFlags) const { return eventFlags & ~flags_; }

    int clear(int eventFlags) const { return static_cast<int>(clear(static_cast<unsigned>(eventFlags))); }
};

const EventFlags EventFlags::TX_PENDING{0x01u};
const EventFlags EventFlags::REMOVE_EVENT{0x02u};
const EventFlags EventFlags::SNAPSHOT_BEGIN{0x04u};
const EventFlags EventFlags::SNAPSHOT_END{0x08u};
const EventFlags EventFlags::SNAPSHOT_SNIP{0x10u};
// 0x20 is reserved. This flag will fit into 1-byte on the wire in QTP protocol
const EventFlags EventFlags::SNAPSHOT_MODE{0x40u};
const EventFlags EventFlags::REMOVE_SYMBOL{0x80u};

} // namespace dxfcpp