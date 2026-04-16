//
// Created by Yeo Shu Heng on 17/4/26.
//

#ifndef COROUTINE_GROUP_HPP
#define COROUTINE_GROUP_HPP

#include <atomic>
#include <boost/asio/detached.hpp>
#include <boost/asio/impl/co_spawn.hpp>
#include <condition_variable>
#include <mutex>

#include "spdlog/spdlog.h"

namespace commons {
template <typename T>
concept void_awaitable = std::same_as<std::decay_t<T>, boost::asio::awaitable<void>>;
class CoroutineGroup {
    std::atomic<int> n_active;
    std::atomic<bool> stopping{false};
    std::mutex m;
    std::condition_variable cv;
    struct coroutine_guard {
        std::atomic<int>& n_active;
        std::condition_variable& cv;
        ~coroutine_guard() {
            n_active.fetch_sub(1, std::memory_order_acq_rel);
            cv.notify_all();
        }
    };

  public:
    template <void_awaitable T> void spawn(boost::asio::io_context& ctx, T&& coroutine) {
        n_active.fetch_add(1, std::memory_order_acq_rel);
        boost::asio::co_spawn(
            ctx,
            [this, cr = std::move(coroutine)]() -> boost::asio::awaitable<void> {
                coroutine_guard guard{n_active, cv};
                co_await cr;
            },
            boost::asio::detached);
    };

    void stop() { stopping.store(true, std::memory_order_release); };

    void join() {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&] { return n_active.load(std::memory_order_acquire) == 0; });
    };

    bool is_stopping() const { return stopping.load(std::memory_order_acquire); };
};
} // namespace commons

#endif // COROUTINE_GROUP_HPP
