//
// Created by Yeo Shu Heng on 16/4/26.
//

#ifndef DISK_HPP
#define DISK_HPP

#include "../wal/wal.hpp"
#include "../core/core.hpp"

namespace core {
class DiskManager final : commons::ThreadHeartBeat {
    io_ctx& ctx;
    wal_ptr wal;
    run_atomic is_running{false};

    timer flush_timer;
    timer hb_timer;
    timer check_timer;

    uint32_t flush_interval;

    boost::asio::awaitable<void> disk_loop();
    boost::asio::awaitable<void> beat_loop() override;
    boost::asio::awaitable<void> check_loop(std::chrono::milliseconds timeout) override;

  public:
    DiskManager(io_ctx& ctx, const wal_ptr& wal, const commons::heartbeat_state& hb_state,
        uint32_t flush_interval);
    ~DiskManager() override;
    void start();
    void shutdown();
};
} // namespace core

#endif // DISK_HPP
