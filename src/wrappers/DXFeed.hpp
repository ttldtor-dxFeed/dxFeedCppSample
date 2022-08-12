#pragma once

#include "DXFCppConfig.hpp"

#include "Connection.hpp"
#include "helpers/Handler.hpp"

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