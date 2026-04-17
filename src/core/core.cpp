//
// Created by Yeo Shu Heng on 14/4/26.
//

#include "../../include/core/core.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace core {
RedisCore::RedisCore(io_ctx& ctx, const size_t max_capacity,
    const commons::heartbeat_state& hb_state, const wal_ptr& wal, const uint32_t poll_interval_ms,
    const uint32_t ttl_interval_ms, const uint32_t ttl_budget)
    : ThreadHeartBeat(hb_state), lru_cache(max_capacity), max_capacity(max_capacity), wal(wal),
      ctx(ctx), poll_interval(poll_interval_ms), ttl_interval(ttl_interval_ms),
      ttl_budget(ttl_budget), poll_timer(ctx), ttl_timer(ctx), hb_timer(ctx) {};

CoreResp RedisCore::execute(command::Command& cmd) {
    return std::visit(
        [this]<typename T>(const T& c) -> CoreResp {
            if constexpr (std::is_same_v<T, command::SetCommand>) {
                lru_cache.add(c.key, core::LRUObject(c.value), c.ttl_ms);
                return CoreResp{CoreResp::RespType::OK, std::nullopt, "OK"};
            } else if constexpr (std::is_same_v<T, command::DelCommand>) {
                if (const bool removed = lru_cache.remove(c.key); !removed) {
                    return CoreResp{CoreResp::RespType::ERROR, std::nullopt, "ERROR"};
                };
                return CoreResp{CoreResp::RespType::OK, std::nullopt, "OK"};
            } else if constexpr (std::is_same_v<T, command::GetCommand>) {
                const auto v = lru_cache.get(c.key);
                if (!v.has_value()) {
                    return CoreResp{CoreResp::RespType::NIL, std::nullopt, "NOT FOUND"};
                }
                return CoreResp{CoreResp::RespType::VALUE, v.value().val, "OK"};
            }
        },
        cmd);
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
    boost::asio::co_spawn(ctx, ttl_loop(), boost::asio::detached);
    boost::asio::co_spawn(ctx, beat_loop(), boost::asio::detached);
};

void RedisCore::shutdown() {
    is_running.store(false);
    boost::system::error_code err;
    ttl_timer.cancel(err);
    hb_timer.cancel(err);
};

boost::asio::awaitable<void> RedisCore::beat_loop() {
    while (is_running.load()) {
        boost::system::error_code ec;
        hb_state->core_heartbeat.store(
            Clock::now().time_since_epoch().count(), std::memory_order_relaxed);
        hb_timer.expires_after(std::chrono::milliseconds(beat_interval_ms));
        co_await hb_timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            if (ec == boost::asio::error::operation_aborted) {
                co_return;
            }
            spdlog::error("unexpected error in redis core heartbeat loop: {}", ec.message());
            co_return;
        }
    }
};
} // namespace core
