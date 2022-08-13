#pragma once

#ifndef DXFEED_HPP_INCLUDED
#error Please include only the DXFeed.hpp header
#endif

#include <memory>
#include <string>

#include "EventFlags.hpp"
#include "EventSource.hpp"

namespace dxfcpp {

struct Entity {
    virtual ~Entity() = default;
    virtual std::string toString() const = 0;
};

struct SharedEntity : public Entity, std::enable_shared_from_this<SharedEntity> {
    using Ptr = std::shared_ptr<SharedEntity>;

    template <typename T> bool is() const {
        try {
            auto p = dynamic_cast<const T *>(this);
            (void)(p);

            return true;
        } catch (const std::bad_cast &) {
            return false;
        }
    }

    template <typename T> std::shared_ptr<T> sharedAs() { return std::dynamic_pointer_cast<T>(shared_from_this()); }

    template <typename T> std::shared_ptr<T> sharedAs() const {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }
};

struct Event : public SharedEntity {
    using Ptr = std::shared_ptr<Event>;

    virtual const std::string &getEventSymbol() const = 0;
    virtual void setEventSymbol(const std::string &) = 0;
    virtual std::uint64_t getEventTime() const = 0;
    virtual void setEventTime(std::uint64_t) = 0;
};

struct MarketEvent : public Event {
    using Ptr = std::shared_ptr<MarketEvent>;

  private:
    std::string eventSymbol_{};
    std::uint64_t eventTime_{};

  protected:
    MarketEvent() : eventSymbol_{}, eventTime_{} {}
    explicit MarketEvent(std::string eventSymbol) : eventSymbol_{std::move(eventSymbol)}, eventTime_{} {}

  public:
    const std::string &getEventSymbol() const override { return eventSymbol_; }
    void setEventSymbol(const std::string &eventSymbol) override { eventSymbol_ = eventSymbol; }
    std::uint64_t getEventTime() const override { return eventTime_; }
    void setEventTime(std::uint64_t eventTime) override { eventTime_ = eventTime; }
};

struct Indexed {
    using Ptr = std::shared_ptr<Indexed>;

    virtual const IndexedEventSource &getSource() const = 0;
    virtual const EventFlagsMask &getEventFlags() const = 0;
    virtual void setEventFlags(const EventFlagsMask &) = 0;
    virtual std::uint64_t getIndex() const = 0;
    virtual void setIndex(std::uint64_t) = 0;
};

struct Lasting {
    using Ptr = std::shared_ptr<Lasting>;
};

struct TimeSeries : public Indexed {
    using Ptr = std::shared_ptr<TimeSeries>;

    virtual std::int64_t getEventId() const { return getIndex(); }
    virtual uint64_t getTime() const = 0;
};

} // namespace dxfcpp