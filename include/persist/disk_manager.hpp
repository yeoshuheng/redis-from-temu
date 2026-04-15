//
// Created by Yeo Shu Heng on 15/4/26.
//

#ifndef DISK_MANAGER_HPP
#define DISK_MANAGER_HPP
#include <boost/asio/awaitable.hpp>

#include "include/command/command.hpp"
#include "include/core/core.hpp"

namespace persist {
enum PersistStrategy { AOF, SNAPSHOT };
class DiskManager : commons::ThreadHeartBeat {
    std::atomic<bool> running{false};
    core::io_ctx& ctx;
    core::timer disk_write_timer;
    uint32_t flush_interval;

  public:
    DiskManager(
        core::io_ctx& ctx, const commons::heartbeat_state& hb_state, const uint32_t flush_interval)
        : ThreadHeartBeat(hb_state), ctx(ctx), disk_write_timer(ctx),
          flush_interval(flush_interval) {};
    virtual ~DiskManager() = default;
    virtual void on_command(command::Command& cmd) = 0;
    virtual boost::asio::awaitable<void> write_to_disk_loop() = 0;
    virtual void load_from_disk(core::RedisCache& lru_cache) = 0;
    void start() {
        running.store(true);
        boost::asio::co_spawn(ctx, write_to_disk_loop(), boost::asio::detached);
    };
    void shutdown() {
        running.store(false);
        boost::system::error_code ec;
        disk_write_timer.cancel(ec);
    };
};
}; // namespace persist

#endif // DISK_MANAGER_HPP
