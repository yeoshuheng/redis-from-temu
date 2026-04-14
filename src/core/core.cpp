//
// Created by Yeo Shu Heng on 14/4/26.
//

#include "../../include/core/core.hpp"

#include <spdlog/spdlog.h>

namespace core {
RedisCore::RedisCore(io_ctx& ctx, const size_t max_capacity,
                     const std::shared_ptr<i_channel>& in_channel,
                     const std::shared_ptr<o_channel>& out_channel, const uint32_t poll_interval_ms,
                     const uint32_t ttl_interval_ms, const uint32_t ttl_budget)
    : lru_cache(max_capacity),
      max_capacity(max_capacity),
      input(in_channel),
      output(out_channel),
      ctx(ctx),
      poll_interval(poll_interval_ms),
      ttl_interval(ttl_interval_ms),
      ttl_budget(ttl_budget),
      poll_timer(ctx),
      ttl_timer(ctx) {};

void RedisCore::execute(command::Command& cmd) {
    std::visit(
        [this]<typename T>(const T& c) {
            if constexpr (std::is_same_v<T, command::SetCommand>) {
                lru_cache.add(c.key, storage::StoredObject(c.value), c.ttl_ms);
            } else if constexpr (std::is_same_v<T, command::DelCommand>) {
                lru_cache.remove(c.key);
            } else if constexpr (std::is_same_v<T, command::GetCommand>) {
                lru_cache.get(c.key);
            }
        },
        cmd);
};

boost::asio::awaitable<void> RedisCore::poll_loop() {
    command::Command cmd;
    while (is_running.load()) {
        boost::system::error_code ec;
        while (input->pop(cmd)) {
            execute(cmd);
        }
        poll_timer.expires_after(std::chrono::milliseconds(poll_interval));
        // instead of throwing error, redirect into an error_code for cleaner handling.
        co_await poll_timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            if (ec == boost::asio::error::operation_aborted) {
                co_return;
            }
            spdlog::error("unexpected error in redis core poll loop: {}", ec.message());
            co_return;
        }
    }
};

boost::asio::awaitable<void> RedisCore::ttl_loop() {
    while (is_running.load()) {
        boost::system::error_code ec;
        // budgeted ttl conviction to prevent hogging event loop from polling commands
        lru_cache.remove_expired(ttl_budget);
        ttl_timer.expires_after(std::chrono::milliseconds(ttl_interval));
        co_await ttl_timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            if (ec == boost::asio::error::operation_aborted) {
                co_return;
            }
            spdlog::error("unexpected error in redis core ttl loop: {}", ec.message());
            co_return;
        }
    }
};

void RedisCore::start() {
    is_running.store(true);
    boost::asio::co_spawn(ctx, poll_loop(), boost::asio::detached);
    boost::asio::co_spawn(ctx, ttl_loop(), boost::asio::detached);
};

void RedisCore::shutdown() {
    is_running.store(false);
    boost::system::error_code err;
    poll_timer.cancel(err);
    ttl_timer.cancel(err);
};
}  // namespace core
