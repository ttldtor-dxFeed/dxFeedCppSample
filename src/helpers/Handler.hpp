#pragma once

#include "wrappers/DXFCppConfig.hpp"

#include <functional>
#include <unordered_map>
#include <mutex>
#include <future>
#include <vector>

namespace dxfcpp {

template <typename Signature> struct Handler;

template <typename... ArgTypes> struct Handler<void(ArgTypes...)> {
    using ListenerType = std::function<void(ArgTypes...)>;

  private:
    mutable std::recursive_mutex listenersMutex_{};
    mutable std::unordered_map<std::size_t, ListenerType> listeners_{};
    std::size_t lastId_{};

  public:
    void handle(ArgTypes &&...args) const {
        std::lock_guard<std::recursive_mutex> guard{listenersMutex_};
        std::vector<std::future<void>> futures{};

        futures.reserve(listeners_.size());

        for (auto &listener : listeners_) {
            futures.emplace_back(std::async(std::launch::async, listener.second, std::forward<ArgTypes>(args)...));
        }

        for (auto &f : futures) {
            f.wait();
        }
    }

    void operator()(ArgTypes &&...args) const { return handle(std::forward<ArgTypes>(args)...); }

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

}