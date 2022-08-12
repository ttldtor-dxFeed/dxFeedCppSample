#pragma once

#include "wrappers/DXFCppConfig.hpp"

#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace dxfcpp {

template <typename Signature> struct Handler;

template <typename... ArgTypes> struct Handler<void(ArgTypes...)> {
    using ListenerType = std::function<void(ArgTypes...)>;

  private:
    std::recursive_mutex listenersMutex_{};
    std::unordered_map<std::size_t, ListenerType> listeners_{};
    std::size_t lastId_{};

    std::recursive_mutex mainFuturesMutex_{};
    std::vector<std::shared_future<void>> mainFutures_{};
    std::size_t mainFuturesCurrentIndex_{};
    std::size_t mainFuturesSize_{};

    std::shared_future<void> handleImpl(ArgTypes &&...args) {
        return std::async(
            std::launch::async,
            [this](ArgTypes &&...args) {
                std::lock_guard<std::recursive_mutex> guard{listenersMutex_};

                for (auto &listener : listeners_) {
                    listener.second(args...);
                }
            },
            std::forward<ArgTypes>(args)...);
    }

  public:
    explicit Handler(std::size_t mainFuturesSize = 256)
        : mainFuturesCurrentIndex_{0ULL}, mainFuturesSize_{mainFuturesSize} {
        mainFutures_.reserve(mainFuturesSize);
    }

    void handle(ArgTypes &&...args) {
        auto f = handleImpl(std::forward<ArgTypes>(args)...);

        {
            std::lock_guard<std::recursive_mutex> guard{mainFuturesMutex_};

            if (mainFutures_.size() < mainFuturesSize_) {
                mainFutures_.template emplace_back(f);
            } else {
                mainFuturesCurrentIndex_ = mainFuturesCurrentIndex_ % mainFuturesSize_;
                mainFutures_[mainFuturesCurrentIndex_].wait();
                mainFutures_[mainFuturesCurrentIndex_] = f;
                mainFuturesCurrentIndex_++;
            }
        }
    }

    void operator()(ArgTypes &&...args) { return handle(std::forward<ArgTypes>(args)...); }

    std::size_t add(ListenerType &&listener) {
        std::lock_guard<std::recursive_mutex> guard{listenersMutex_};

        lastId_++;
        listeners_.template emplace(lastId_, std::forward<ListenerType>(listener));

        return lastId_;
    }

    std::size_t operator+=(ListenerType &&listener) { return add(std::forward<ListenerType>(listener)); }

    void remove(std::size_t id) {
        std::lock_guard<std::recursive_mutex> guard{listenersMutex_};

        if (listeners_.count(id) > 0) {
            listeners_.erase(id);
        }
    }

    void operator-=(std::size_t id) { return remove(id); }
};

} // namespace dxfcpp