//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef CORE_HPP
#define CORE_HPP

#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "../command/parser.hpp"
#include "../commons/heartbeat.hpp"
#include "../wal/wal.hpp"
#include "lru_cache.hpp"
#include "object.hpp"
#include "resp.hpp"

namespace core {
using RedisCache = LRUCache<std::string, core::LRUObject>;
using io_ctx = boost::asio::io_context;
using timer = boost::asio::steady_timer;
using wal_ptr = std::shared_ptr<wal::WAL>;
using run_atomic = std::atomic<bool>;
class RedisCore final : commons::ThreadHeartBeat {
  private:
    RedisCache lru_cache;
    size_t max_capacity;
    wal_ptr wal;

    run_atomic is_running{false};
    io_ctx& ctx;
    uint32_t poll_interval;
    uint32_t ttl_interval;
    uint32_t ttl_budget;
    timer poll_timer;
    timer ttl_timer;
    timer hb_timer;

    boost::asio::awaitable<void> ttl_loop();
    boost::asio::awaitable<void> beat_loop() override;

  public:
    explicit RedisCore(io_ctx& ctx, size_t max_capacity, const commons::heartbeat_state& hb_state,
        const wal_ptr& wal, uint32_t poll_interval_ms, uint32_t ttl_interval_ms,
        uint32_t ttl_budget);
    CoreResp execute(command::Command& cmd);
    void start();
    void shutdown();
};
} // namespace core

#endif // CORE_HPP
