#pragma once

#include "Connection.hpp"

#include <memory>
#include <string>

namespace dxfcs {
namespace DXFeed {
inline std::shared_ptr<dxfcs::Connection> connect(const std::string &address) { return Connection::create(address); }
} // namespace DXFeed

} // namespace dxfcs