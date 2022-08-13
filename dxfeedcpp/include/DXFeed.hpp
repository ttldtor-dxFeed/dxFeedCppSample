#pragma once

#define DXFEED_HPP_INCLUDED 1

#include "common/DXFCppConfig.hpp"

#include "connections/Connection.hpp"
#include "connections/ConnectionStatus.hpp"

#include "converters/DateTimeConverter.hpp"
#include "converters/StringConverter.hpp"

#include "events/Event.hpp"
#include "events/Candle.hpp"
#include "events/EventFlags.hpp"
#include "events/EventSource.hpp"
#include "events/EventTraits.hpp"
#include "events/EventType.hpp"
#include "events/OrderScope.hpp"
#include "events/Quote.hpp"

#include "helpers/Handler.hpp"
#include "helpers/IdGenerator.hpp"
#include "helpers/LogDumper.hpp"

#include "processors/AbstractEventCheckingProcessor.hpp"
#include "processors/AbstractEventProcessor.hpp"
#include "processors/CompositeProcessor.hpp"

#include "subscriptions/Subscription.hpp"

#include "tools/EventsCollector.hpp"

#include <memory>
#include <string>

namespace dxfcpp {

namespace DXFeed {

template <typename OnDisconnectListener = typename Handler<void()>::ListenerType,
          typename OnConnectionStatusChangedListener =
              typename Handler<void(ConnectionStatus, ConnectionStatus)>::ListenerType>
inline Connection::Ptr connect(const std::string &address, OnDisconnectListener &&onDisconnectListener,
                               OnConnectionStatusChangedListener &&onConnectionStatusChangedListener) {
    return Connection::create(address, std::forward<OnDisconnectListener>(onDisconnectListener),
                              std::forward<OnConnectionStatusChangedListener>(onConnectionStatusChangedListener));
}

inline Connection::Ptr connect(const std::string &address) { return Connection::create(address); }
} // namespace DXFeed

} // namespace dxfcpp