#pragma once

#include <DXFeed.h>
#include <EventData.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "DXFCppConfig.hpp"

#include "EventType.hpp"

namespace dxfcpp {

struct Connection;

struct Subscription {
    dxf_subscription_t subscriptionHandle_ = nullptr;

    static std::shared_ptr<Subscription> create(std::weak_ptr<Connection> connection,
                                                const std::vector<EventType> &eventTypes);

    void addSymbol(const std::string &symbol);

    void removeSymbol(const std::string &symbol);

    void addSymbols(const std::vector<std::string> &symbols);

    void removeSymbols(const std::vector<std::string> &symbols);

    void clearSymbols();
};

} // namespace dxfcpp