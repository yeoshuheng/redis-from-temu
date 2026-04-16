//
// Created by Yeo Shu Heng on 16/4/26.
//

#ifndef DISK_HPP
#define DISK_HPP

#include "../../include/commons/coroutine_group.hpp"
#include "../core/core.hpp"
#include "state.hpp"

namespace disk {
using core::Clock;
using core::io_ctx;
using core::timer;
using core::wal_ptr;
class DiskManager final : commons::ThreadHeartBeat {
    io_ctx& ctx;
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
    DiskManager(io_ctx& ctx, const wal_ptr& wal, const commons::heartbeat_state& hb_state,
        uint32_t flush_interval);
    ~DiskManager() override;
    void start();
    void shutdown();
};
} // namespace disk

#endif // DISK_HPP
