#pragma once

#include "Connection.hpp"
#include "helpers/Handler.hpp"

#include <memory>
#include <string>

namespace dxfcs {
namespace DXFeed {

template <typename OnDisconnectListener = typename Handler<void()>::ListenerType,
          typename OnConnectionStatusChangedListener =
              typename Handler<void(const ConnectionStatus &, const ConnectionStatus &)>::ListenerType>
inline std::shared_ptr<Connection> connect(const std::string &address, OnDisconnectListener &&onDisconnectListener,
                                           OnConnectionStatusChangedListener &&onConnectionStatusChangedListener) {
    return Connection::create(address, std::forward<OnDisconnectListener>(onDisconnectListener),
                              std::forward<OnConnectionStatusChangedListener>(onConnectionStatusChangedListener));
}

inline std::shared_ptr<Connection> connect(const std::string &address) { return Connection::create(address); }
} // namespace DXFeed

} // namespace dxfcs