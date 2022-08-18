#pragma once

#define DXFEED_HPP_INCLUDED 1

#include "common/DXFCppConfig.hpp"

#include "utils/Utils.hpp"

#include "connections/Connection.hpp"
#include "connections/ConnectionStatus.hpp"

#include "converters/DateTimeConverter.hpp"
#include "converters/StringConverter.hpp"

#include "events/Candle.hpp"
#include "events/Configuration.hpp"
#include "events/Direction.hpp"
#include "events/Event.hpp"
#include "events/EventFlags.hpp"
#include "events/EventSource.hpp"
#include "events/EventTraits.hpp"
#include "events/EventType.hpp"
#include "events/Greeks.hpp"
#include "events/Order.hpp"
#include "events/OrderScope.hpp"
#include "events/PriceType.hpp"
#include "events/Profile.hpp"
#include "events/Quote.hpp"
#include "events/Series.hpp"
#include "events/Summary.hpp"
#include "events/TheoPrice.hpp"
#include "events/TimeAndSale.hpp"
#include "events/Trade.hpp"
#include "events/Underlying.hpp"

#include "helpers/Handler.hpp"
#include "helpers/IdGenerator.hpp"
#include "helpers/LogDumper.hpp"

#include "processors/AbstractEventCheckingProcessor.hpp"
#include "processors/AbstractEventProcessor.hpp"
#include "processors/CompositeProcessor.hpp"

#include "subscriptions/Subscription.hpp"

#include <memory>
#include <string>

namespace dxfcpp {

/// The main "anchor"
namespace DXFeed {

/**
 * Creates the new connection to specified address with specified onDisconnect & onConnectionStatusChanged listeners
 *
 * @tparam OnDisconnectListener The type of a onDisconnect listener (it could be any callable)
 * @tparam OnConnectionStatusChangedListener The type of onConnectionStatusChanged listener
 * @param address The address to connect
 * @param onDisconnectListener The onDisconnect listener
 * @param onConnectionStatusChangedListener The onConnectionStatusChanged listener
 * @return A shared pointer to the new connection object or Connection::INVALID
 */
template <typename OnDisconnectListener = typename Handler<void()>::ListenerType,
          typename OnConnectionStatusChangedListener =
              typename Handler<void(ConnectionStatus, ConnectionStatus)>::ListenerType>
inline Connection::Ptr connect(const std::string &address, OnDisconnectListener &&onDisconnectListener,
                               OnConnectionStatusChangedListener &&onConnectionStatusChangedListener) {
    return Connection::create(address, std::forward<OnDisconnectListener>(onDisconnectListener),
                              std::forward<OnConnectionStatusChangedListener>(onConnectionStatusChangedListener));
}

/**
 * Creates the new connection to specified address
 *
 * @param address The address to connect
 * @return A shared pointer to the new connection object or Connection::INVALID
 */
inline Connection::Ptr connect(const std::string &address) { return Connection::create(address); }
} // namespace DXFeed

} // namespace dxfcpp