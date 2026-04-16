//
// Created by Yeo Shu Heng on 16/4/26.
//
#include "../../include/core/disk.hpp"

namespace core {
DiskManager::DiskManager(io_ctx& ctx, const wal_ptr& wal, const commons::heartbeat_state& hb_state,
    const uint32_t flush_interval)
    : ThreadHeartBeat(hb_state), ctx(ctx), wal(wal), flush_timer(ctx), hb_timer(ctx),
      check_timer(ctx), flush_interval(flush_interval) {};

DiskManager::~DiskManager() {
    shutdown();
};

boost::asio::awaitable<void> DiskManager::beat_loop() {
    while (is_running.load(std::memory_order_acquire)) {
        boost::system::error_code ec;
        hb_state->disk_heartbeat.store(Clock::now(), std::memory_order_relaxed);
        hb_timer.expires_after(std::chrono::milliseconds(beat_interval_ms));
        co_await hb_timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            if (ec == boost::asio::error::operation_aborted) {
                co_return;
            }
            spdlog::error("unexpected error in disk manager heartbeat loop: {}", ec.message());
            co_return;
        }
    }
};

boost::asio::awaitable<void> DiskManager::check_loop(const std::chrono::milliseconds timeout) {
    while (is_running.load(std::memory_order_acquire)) {
        boost::system::error_code ec;
        if (const auto last_core_hb = hb_state->core_heartbeat.load(std::memory_order_relaxed);
            Clock::now() - last_core_hb > timeout) {
            spdlog::error("redis core timed out, last heartbeat: {}",
                last_core_hb.time_since_epoch().count());
        }
        check_timer.expires_after(std::chrono::milliseconds(check_interval_ms));
        co_await check_timer.async_wait(
            boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            if (ec == boost::asio::error::operation_aborted) {
                co_return;
            }
            spdlog::error(
                "unexpected error in disk manager heartbeat check loop: {}", ec.message());
            co_return;
        }
    }
};

boost::asio::awaitable<void> DiskManager::disk_loop() {
    while (is_running.load(std::memory_order_acquire)) {
        boost::system::error_code ec;
        wal->flush();
        flush_timer.expires_after(std::chrono::milliseconds(flush_interval));
        co_await flush_timer.async_wait(
            boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            if (ec == boost::asio::error::operation_aborted) {
                co_return;
            }
            spdlog::error("unexpected error in disk manager flush loop: {}", ec.message());
            co_return;
        }
    }
};

void DiskManager::start() {
    is_running.store(true, std::memory_order_release);
    boost::asio::co_spawn(ctx, disk_loop(), boost::asio::detached);
    boost::asio::co_spawn(ctx, beat_loop(), boost::asio::detached);
    boost::asio::co_spawn(ctx, check_loop(std::chrono::milliseconds(100)), boost::asio::detached);
};

void DiskManager::shutdown() {
    is_running.store(false, std::memory_order_release);
    boost::system::error_code err;
    flush_timer.cancel(err);
    check_timer.cancel(err);
    hb_timer.cancel(err);
};

} // namespace core
