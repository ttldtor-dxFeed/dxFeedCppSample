#pragma once

#include <DXFeed.h>
#include <EventData.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "DateTimeConverter.hpp"
#include "EventFlag.hpp"
#include "StringConverter.hpp"

namespace dxfcs {

class EventsCollector {
    template <typename EventType> class SnapshotHolder {
        std::atomic<bool> disconnected_{false};
        std::atomic<bool> done_{false};

        std::mutex eventsMutex_{};
        std::map<typename EventType::IndexType, EventType> events_{};

        std::mutex cvMutex_{};
        std::condition_variable cv_{};

        std::string symbol_;
        long long fromTime_;
        long long toTime_;

      public:
        SnapshotHolder(std::string symbol, long long fromTime, long long toTime)
            : symbol_{std::move(symbol)}, fromTime_{fromTime}, toTime_{toTime} {}

        void wait(long long timeout) {
            std::unique_lock<std::mutex> lk(cvMutex_);
            if (timeout == 0) {
                cv_.wait(lk, [this] { return disconnected_ || done_; });
            } else {
                cv_.wait_for(lk, std::chrono::milliseconds(timeout), [this] { return disconnected_ || done_; });
            }
        }

        void onDisconnect() {
            disconnected_ = true;
            cv_.notify_one();
        }

        void done() {
            done_ = true;
            cv_.notify_one();
        }

        void applyEventData(const dxf_event_data_t *eventData) {
            // We need a copy
            //  TODO: std::bit_cast C++20
            auto event = EventType(symbol_, *reinterpret_cast<const typename EventType::RawType *>(eventData));

            // std::cout << event.toString() << std::endl;

            std::lock_guard<std::mutex> guard(eventsMutex_);

            if (event.getTime() >= fromTime_ && event.getTime() <= toTime_) {
                bool remove = dxfcs::EventFlag::REMOVE_EVENT.in(event.getEventFlags());

                event.setEventFlags(0u);

                auto found = events_.find(event.getIndex());

                if (found == events_.end()) {
                    if (!remove) {
                        events_.emplace(event.getIndex(), event);
                    }
                } else if (remove) {
                    events_.erase(found);
                } else {
                    found->second = event;
                }
            }

            if (event.getTime() <= fromTime_ || dxfcs::EventFlag::SNAPSHOT_SNIP.in(event.getEventFlags())) {
                done();
            }
        }

        std::vector<EventType> getResult() {
            std::lock_guard<std::mutex> guard(eventsMutex_);

            std::vector<EventType> result{};

            // reverse
            std::transform(events_.rbegin(), events_.rend(), std::back_inserter(result),
                           [](const std::pair<typename EventType::IndexType, EventType> &pair) { return pair.second; });

            return result;
        }
    };

  public:
    template <typename EventType>
    static void onEventHandler(int eventType, const dxf_event_data_t *eventData, void *userData) {
        if (eventType == EventType::EVENT_TYPE) {
            static_cast<SnapshotHolder<EventType> *>(userData)->applyEventData(eventData);
        }
    }

    template <typename EventType>
    std::future<std::vector<EventType>> collectTimeSeriesSnapshot(const std::string &address, const std::string &symbol,
                                                                  long long fromTime, long long toTime, long timeout) {
        return std::async(
            std::launch::async,
            [](const std::string &address, const std::string &symbol, long long fromTime, long long toTime,
               long timeout) -> std::vector<EventType> {
                SnapshotHolder<EventType> holder{symbol, fromTime, toTime};

                dxf_connection_t con = nullptr;
                auto r = dxf_create_connection(
                    address.c_str(),
                    [](dxf_connection_t, void *data) {
                        static_cast<SnapshotHolder<EventType> *>(data)->onDisconnect();
                    },
                    nullptr, nullptr, nullptr, static_cast<void *>(&holder), &con);

                if (r == DXF_FAILURE) {
                    return {};
                }

                dxf_subscription_t sub = nullptr;

                r = dxf_create_subscription_timed(con, EventType::EVENT_TYPE, fromTime, &sub);

                if (r == DXF_FAILURE) {
                    dxf_close_connection(con);

                    return {};
                }

                auto eventListener = [](int eventType, dxf_const_string_t /* symbolName */,
                                        const dxf_event_data_t *eventData, int /*dataCount (always 1) */,
                                        void *userData) {
                    return onEventHandler<EventType>(eventType, eventData, userData);
                };

                r = dxf_attach_event_listener(sub, eventListener, static_cast<void *>(&holder));

                if (r == DXF_FAILURE) {
                    dxf_close_subscription(sub);
                    dxf_close_connection(con);

                    return {};
                }

                std::wstring wideSymbol = dxfcs::StringConverter::utf8ToWString(symbol);
                r = dxf_add_symbol(sub, wideSymbol.c_str());

                if (r == DXF_FAILURE) {
                    dxf_detach_event_listener(sub, eventListener);
                    dxf_close_subscription(sub);
                    dxf_close_connection(con);

                    return {};
                }

                holder.wait(timeout);

                dxf_detach_event_listener(sub, eventListener);
                dxf_close_subscription(sub);
                dxf_close_connection(con);

                return holder.getResult();
            },
            address, symbol, fromTime, toTime, timeout);
    }
};

} // namespace dxfcs