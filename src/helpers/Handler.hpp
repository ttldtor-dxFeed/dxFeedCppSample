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
        return std::async(std::launch::async, [this, &args...] {
#ifndef NDEBUG
            dump("handleImpl::async::begin", this);
#endif
            std::lock_guard<std::recursive_mutex> guard{listenersMutex_};
            std::vector<std::future<void>> futures{};

            futures.reserve(listeners_.size());

            for (auto &listener : listeners_) {
                futures.emplace_back(std::async(std::launch::async, listener.second, std::forward<ArgTypes>(args)...));
            }

            for (auto &f : futures) {
                f.wait();
            }
#ifndef NDEBUG
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            dump("handleImpl::async::end", this);
#endif
        });
    }

  public:
    explicit Handler(std::size_t mainFuturesSize = 256)
        : mainFuturesCurrentIndex_{0ULL}, mainFuturesSize_{mainFuturesSize} {
        mainFutures_.reserve(mainFuturesSize);
    }

#ifndef NDEBUG
    template <typename T, typename This> static void dump(T &&v, This&&) {
        std::ostringstream oss{};

        std::string pf = __PRETTY_FUNCTION__;

        std::string::size_type found = pf.find("This = ");
        std::string clazz{};
        if (found != std::string::npos) {
            std::string::size_type found2 = pf.find("*;", found + 7);

            if (found2 != std::string::npos) {
                clazz = pf.substr(found + 7, found2 - (found + 7));
            }
        }

        oss << "[" << clazz << "][" << std::this_thread::get_id() << "] (" << v << "): "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
                   .count()
            << "\n";

        std::cout << oss.str();
    }
#endif

    void handle(ArgTypes &&...args) {
#ifndef NDEBUG
        dump("handle::begin", this);
#endif

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

#ifndef NDEBUG
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        dump("handle::end", this);
#endif
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