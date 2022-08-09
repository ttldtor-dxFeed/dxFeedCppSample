#pragma once

#include "wrappers/DXFCppConfig.hpp"

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

namespace dxfcpp {

template <int...> struct integerSequence {};

template <int N, int... Ns> struct generateIntegerSequence : generateIntegerSequence<N - 1, N - 1, Ns...> {};

template <int... Ns> struct generateIntegerSequence<0, Ns...> {
    typedef integerSequence<Ns...> Type;
};

template <typename Signature> struct Handler;

template <typename... ArgTypes> struct Handler<void(ArgTypes...)> {
    using ListenerType = std::function<void(ArgTypes...)>;

    class Executor {
        class Task {
            std::size_t uid_{};
            std::size_t priority_{};
            std::tuple<ListenerType &&> listener_{};
            std::tuple<ArgTypes &&...> payload_{};

          public:
            template <typename L, typename P>
            Task(std::size_t uid, std::size_t priority, L &&listener, P &&payload)
                : uid_{uid}, priority_{priority}, listener_{std::forward<L>(listener)}, payload_{
                                                                                            std::forward<P>(payload)} {}

            std::size_t getUid() const { return uid_; }

            std::size_t getPriority() const { return priority_; }

            std::tuple<ListenerType &&> &getListener() { return listener_; }

            std::tuple<ArgTypes &&...> &getPayload() { return payload_; }

            std::size_t hash() const noexcept { return uid_; }

            friend bool operator<(const Task &t1, const Task &t2) { return t1.priority_ < t2.priority_; }

            friend bool operator<(const std::shared_ptr<Task> &t1, const std::shared_ptr<Task> &t2) {
                return t1->getPriority() < t2.getPriority();
            }
        };

        std::atomic<bool> needToStop_{false};
        std::atomic<bool> stopped_{false};
        std::atomic<bool> hasData_{false};
        std::atomic<bool> bulkOperation_{false};

        std::mutex tasksMutex_{};
        std::condition_variable cv_{};
        std::multimap<std::size_t, std::shared_ptr<Task>> tasks_{};
        std::unordered_set<std::size_t> uids_{};

        std::unique_ptr<std::thread> mainThread_{};

        template <int... Ns> void executeImpl(integerSequence<Ns...>) {
            std::lock_guard<std::mutex> guard(tasksMutex_);
            std::vector<std::future<void>> futures{};

            futures.reserve(tasks_.size());

            for (auto taskIt = tasks_.rbegin(); taskIt != tasks_.rend(); taskIt++) {
                futures.emplace_back(std::async(std::launch::async,
                                                std::forward<ListenerType>(std::get<0>(taskIt->second->getListener())),
                                                std::get<Ns>(taskIt->second->getPayload())...));

                uids_.erase(taskIt->second->getUid());
            }

            for (auto &f : futures) {
                f.wait();
            }

            tasks_.clear();
        }

        void wait(unsigned long timeout) noexcept {
            std::unique_lock<std::mutex> lk(tasksMutex_);

            if (timeout == 0) {
                cv_.wait(lk, [this] { return (hasData_ && !bulkOperation_) || needToStop_; });
            } else {
                cv_.wait_for(lk, std::chrono::milliseconds(timeout),
                             [this] { return (hasData_ && !bulkOperation_) || needToStop_; });
            }
        }

      public:
        explicit Executor(bool valid = true, unsigned long timeout = 0UL) {
            if (!valid) {
                return;
            }

            mainThread_.reset(new std::thread([&] {
                while (!needToStop_) {
                    wait(timeout);

                    if (hasData_) {
                        executeImpl(typename generateIntegerSequence<sizeof...(ArgTypes)>::Type{});

                        hasData_ = false;
                    }
                }

                stopped_ = true;
            }));
        }

        void beginBulk() { bulkOperation_ = true; }

        void endBulk() {
            bulkOperation_ = false;

            if (hasData_) {
                cv_.notify_all();
            }
        }

        void addTask(std::size_t uid, std::size_t priority, ListenerType &&listener, ArgTypes &&...args) noexcept {
            if (!mainThread_) {
                return;
            }

            {
                std::lock_guard<std::mutex> guard(tasksMutex_);

                if (uids_.count(uid) > 0) {
                    return;
                }

                tasks_.emplace(priority,
                               std::make_shared<Task>(uid, priority,
                                                      std::forward_as_tuple(std::forward<ListenerType>(listener)),
                                                      std::forward_as_tuple(std::forward<ArgTypes>(args)...)));
                uids_.emplace(uid);
                hasData_ = true;
            }

            if (!bulkOperation_) {
                cv_.notify_all();
            }
        }

        ~Executor() {
            if (!stopped_) {
                needToStop_ = true;

                mainThread_->join();
            }
        }
    };

  private:
    mutable std::recursive_mutex listenersMutex_{};
    mutable std::unordered_map<std::size_t, ListenerType> listeners_{};
    std::size_t lastId_{};
    Executor executor_{};

  public:
    explicit Handler(bool valid = true) : executor_(valid) {}

    void handle(ArgTypes &&...args) {
        std::lock_guard<std::recursive_mutex> guard{listenersMutex_};

        if (listeners_.empty()) {
            return;
        }

        //executor_.beginBulk();
        for (auto &listener : listeners_) {
            executor_.addTask(listener.first, 1, std::forward<ListenerType>(listener.second),
                              std::forward<ArgTypes>(args)...);
        }
        //executor_.endBulk();
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