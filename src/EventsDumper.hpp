#pragma once

#include <condition_variable>
#include <future>
#include <mutex>
#include <vector>

namespace dxfcs {

class EventsDumper {
    class State {
        std::atomic<bool> disconnected_{false};
        std::mutex cvMutex_{};
        std::condition_variable cv_{};

      public:
        void wait(long long timeout) {
            std::unique_lock<std::mutex> lk(cvMutex_);
            if (timeout == 0) {
                cv_.wait(lk, [this] { return disconnected_.load(); });
            } else {
                cv_.wait_for(lk, std::chrono::milliseconds(timeout), [this] { return disconnected_.load(); });
            }
        }

        void onDisconnect() {
            disconnected_ = true;
            cv_.notify_one();
        }
    };

  public:
    template <typename EventType>
    static void onEventHandler(int eventType, dxf_const_string_t symbolName, const dxf_event_data_t *eventData) {
        if (eventType == EventType::EVENT_TYPE) {
            auto event = EventType(StringConverter::wStringToUtf8(symbolName),
                                   *reinterpret_cast<const typename EventType::RawType *>(eventData));

            // TODO: std::osyncstream (C++20)
            std::cout << event.toString() << std::endl;
        }
    }

    template <typename EventType>
    std::future<void> dumpEvents(const std::string &address, const std::vector<std::string> &symbols, long timeout) {
        return std::async(
            std::launch::async,
            [](const std::string &address, const std::vector<std::string> &symbols, long timeout) -> void {
                State state{};

                dxf_connection_t con = nullptr;
                auto r = dxf_create_connection(
                    address.c_str(), [](dxf_connection_t, void *data) { static_cast<State *>(data)->onDisconnect(); },
                    nullptr, nullptr, nullptr, static_cast<void *>(&state), &con);

                if (r == DXF_FAILURE) {
                    return;
                }

                dxf_subscription_t sub = nullptr;

                r = dxf_create_subscription(con, EventType::EVENT_TYPE, &sub);

                if (r == DXF_FAILURE) {
                    dxf_close_connection(con);

                    return;
                }

                auto eventListener = [](int eventType, dxf_const_string_t symbolName, const dxf_event_data_t *eventData,
                                        int /*dataCount (always 1) */, void * /*userData*/) {
                    return onEventHandler<EventType>(eventType, symbolName, eventData);
                };

                r = dxf_attach_event_listener(sub, eventListener, nullptr);

                if (r == DXF_FAILURE) {
                    dxf_close_subscription(sub);
                    dxf_close_connection(con);

                    return;
                }

                std::vector<std::wstring> wSymbols(symbols.size());
                std::transform(symbols.begin(), symbols.end(), wSymbols.begin(),
                               [](const std::string &s) { return StringConverter::utf8ToWString(s); });

                for (const auto &ws : wSymbols) {
                    r = dxf_add_symbol(sub, ws.c_str());

                    if (r == DXF_FAILURE) {
                        dxf_detach_event_listener(sub, eventListener);
                        dxf_close_subscription(sub);
                        dxf_close_connection(con);

                        return;
                    }
                }

                state.wait(timeout);

                dxf_detach_event_listener(sub, eventListener);
                dxf_close_subscription(sub);
                dxf_close_connection(con);
            },
            address, symbols, timeout);
    }
};

} // namespace dxfcs