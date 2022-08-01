#pragma once

#include <memory>

#include <DXFeed.h>
#include <string>

namespace dxfcs {

class Connection final : public std::enable_shared_from_this<Connection> {
    dxf_connection_t con = nullptr;

    void onDisconnect() {}

    void onStatusChanged() {}

  public:
    ~Connection() {
        if (con != nullptr) {
            dxf_close_connection(con);
        }
    }

    static std::shared_ptr<Connection> create(const std::string &address) {
        auto c = std::make_shared<Connection>();

        dxf_connection_t con = nullptr;
        auto r = dxf_create_connection(
            address.c_str(),
            [](dxf_connection_t, void *data) { static_cast<Connection *>(data)->shared_from_this()->onDisconnect(); },
            [](dxf_connection_t, dxf_connection_status_t, dxf_connection_status_t, void *data) {
                static_cast<Connection *>(data)->shared_from_this()->onStatusChanged();
            },
            nullptr, nullptr, static_cast<void *>(c.get()), &con);

        if (r == DXF_FAILURE) {
            return {};
        }

        //TODO: implement handlers, logging, callbacks
        return c;
    }
};

} // namespace dxfcs