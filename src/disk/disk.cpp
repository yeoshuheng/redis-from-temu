//
// Created by Yeo Shu Heng on 16/4/26.
//
#include "../../include/disk/disk.hpp"

namespace disk {
DiskManager::DiskManager(
    const wal_ptr& wal, const commons::heartbeat_state& hb_state, const uint32_t flush_interval)
    : ThreadHeartBeat(hb_state), wal(wal), flush_timer(disk_ctx), hb_timer(disk_ctx),
      check_timer(disk_ctx), flush_interval(flush_interval) {};

DiskManager::~DiskManager() {
    shutdown();
};

bool DiskManager::is_running() const {
    return state.load(std::memory_order_acquire) == DiskManagerState::RUNNING;
};

bool DiskManager::is_beating() const {
    const DiskManagerState curr_state = state.load(std::memory_order_acquire);
    return curr_state == DiskManagerState::RUNNING ||
           curr_state == DiskManagerState::STOP_REQUESTED;
};

boost::asio::awaitable<void> DiskManager::beat_loop() {
    while (is_beating()) {
        boost::system::error_code ec;
        hb_state->disk_heartbeat.store(
            Clock::now().time_since_epoch().count(), std::memory_order_relaxed);
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

boost::asio::awaitable<void> DiskManager::disk_loop() {
    while (true) {
        const DiskManagerState curr_state = state.load(std::memory_order_acquire);
        boost::system::error_code ec;
        if (curr_state == DiskManagerState::STOPPED) { // stopped can return
            co_return;
        }
        if (curr_state ==
            DiskManagerState::STOP_REQUESTED) { // try to flush whatever is left behind in WAL
            wal->flush();
            if (wal->is_empty()) {
                co_return; // can terminate coroutine, WAL has flushed fully
            }
            flush_timer.expires_after(std::chrono::milliseconds(1));
            co_await flush_timer.async_wait(
                boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            continue;
        };
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
    state.store(DiskManagerState::RUNNING, std::memory_order_release);
    group.spawn(
        disk_ctx, [&]() -> boost::asio::awaitable<void> { co_return co_await disk_loop(); });
    group.spawn(
        disk_ctx, [&]() -> boost::asio::awaitable<void> { co_return co_await beat_loop(); });
    disk_thread = std::thread([this] { disk_ctx.run(); });
};

void DiskManager::shutdown() {
    state.store(DiskManagerState::STOP_REQUESTED, std::memory_order_release);
    group.stop();
    boost::system::error_code err;
    flush_timer.cancel(err);
    check_timer.cancel(err);
    hb_timer.cancel(err);
    group.join();
    state.store(DiskManagerState::STOPPED, std::memory_order_release);
};

} // namespace disk
