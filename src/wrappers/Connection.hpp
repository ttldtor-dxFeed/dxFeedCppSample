#pragma once

#include <memory>

#include <DXFeed.h>
#include <string>
#include <vector>
#include <functional>
#include <future>

namespace dxfcs {

struct Connection final : public std::enable_shared_from_this<Connection> {
    using OnDisconnectListenerType = std::function<void()>;

  private:
    dxf_connection_t con = nullptr;
    std::vector<OnDisconnectListenerType> onDisconnectListeners {};

    void onDisconnect() {
        std::vector<std::future<void>> futures{onDisconnectListeners.size()};

        for (auto&& disconnectListener : onDisconnectListeners) {
            futures.emplace_back(std::async(std::launch::async, std::forward<OnDisconnectListenerType>(disconnectListener)));
        }

        for (auto&& future : futures) {
            future.get();
        }
    }

    void onStatusChanged() {}


  public:
    ~Connection() {
        if (con != nullptr) {
            dxf_close_connection(con);
        }
    }

    template <typename Listener = OnDisconnectListenerType>
    void addOnDisconnectListener(Listener&& listener) {
        onDisconnectListeners.emplace_back(std::forward<Listener>(listener));
    }

    template <typename OnDisconnectListener = OnDisconnectListenerType>
    static std::shared_ptr<Connection> create(const std::string &address, OnDisconnectListener&& disconnectListener) {
        auto c = std::make_shared<Connection>();

        c->template addOnDisconnectListener<>(std::forward<OnDisconnectListener>(disconnectListener));

        dxf_connection_t con = nullptr;
        auto r = dxf_create_connection(
            address.c_str(),
            [](dxf_connection_t, void *data) { reinterpret_cast<Connection *>(data)->shared_from_this()->onDisconnect(); },
            [](dxf_connection_t, dxf_connection_status_t, dxf_connection_status_t, void *data) {
                reinterpret_cast<Connection *>(data)->onStatusChanged();
            },
            nullptr, nullptr, reinterpret_cast<void *>(c.get()), &con);

        if (r == DXF_FAILURE) {
            return {};
        }

        //TODO: implement handlers, logging, callbacks
        return c;
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

/*
{
    dxfcs::Connection::create("demo.dxfeed.com:7300", [](){
       std::cout << "Disconnected" << std::endl;
    });

std::this_thread::sleep_for(std::chrono::seconds(5));
}
 */