//
// Created by Yeo Shu Heng on 16/4/26.
//

#ifndef DISK_HPP
#define DISK_HPP

#include "../../include/commons/coroutine_group.hpp"
#include "../commons/heartbeat.hpp"
#include "../core/core.hpp"
#include "state.hpp"

namespace disk {
using Clock = std::chrono::steady_clock;
using io_ctx = boost::asio::io_context;
using timer = boost::asio::steady_timer;
using wal_ptr = std::shared_ptr<wal::WAL>;
class DiskManager final : commons::ThreadHeartBeat {
    io_ctx disk_ctx;
    wal_ptr wal;
    std::atomic<DiskManagerState> state{DiskManagerState::STOPPED};
    commons::CoroutineGroup group{};

    timer flush_timer;
    timer hb_timer;
    timer check_timer;

    uint32_t flush_interval;

    boost::asio::awaitable<void> disk_loop();
    boost::asio::awaitable<void> beat_loop() override;
    bool is_running() const;
    bool is_beating() const;

  public:
    DiskManager(
        const wal_ptr& wal, const commons::heartbeat_state& hb_state, uint32_t flush_interval);
    ~DiskManager() override;
    void start();
    void shutdown();
};
} // namespace disk

#endif // DISK_HPP
