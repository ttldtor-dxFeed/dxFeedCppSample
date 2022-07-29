#pragma once

#include <memory>

namespace dxfcs {

struct Connection : std::enable_shared_from_this<Connection> {
    static std::shared_ptr<Connection> create(const std::string &address) { return {}; }
};

} // namespace dxfcs