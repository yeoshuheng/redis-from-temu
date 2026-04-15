//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef CORE_HPP
#define CORE_HPP

#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "../command/parser.hpp"
#include "../commons/heartbeat.hpp"
#include "lru_cache.hpp"
#include "object.hpp"
#include "resp.hpp"

namespace core {
using RedisCache = LRUCache<std::string, core::LRUObject>;
using io_ctx = boost::asio::io_context;
using timer = boost::asio::steady_timer;
using run_atomic = std::atomic<bool>;
class RedisCore final : commons::ThreadHeartBeat {
    using i_channel = boost::lockfree::spsc_queue<command::Command>;
    using o_channel = boost::lockfree::spsc_queue<Response>;

  private:
    RedisCache lru_cache;
    size_t max_capacity;
    std::shared_ptr<i_channel> input;
    std::shared_ptr<o_channel> output;
    run_atomic is_running{false};

    io_ctx& ctx;
    uint32_t poll_interval;
    uint32_t ttl_interval;
    uint32_t ttl_budget;
    timer poll_timer;
    timer ttl_timer;
    timer hb_timer;
    timer check_timer;

    boost::asio::awaitable<void> poll_loop();
    boost::asio::awaitable<void> ttl_loop();
    void execute(command::Command& cmd);
    boost::asio::awaitable<void> beat_loop() override;
    boost::asio::awaitable<void> check_loop(std::chrono::milliseconds timeout) override;

  public:
    explicit RedisCore(io_ctx& ctx, size_t max_capacity, const commons::heartbeat_state& hb_state,
        const std::shared_ptr<i_channel>& in_channel, const std::shared_ptr<o_channel>& out_channel,
        uint32_t poll_interval_ms, uint32_t ttl_interval_ms, uint32_t ttl_budget);
    void start();
    void shutdown();
};
} // namespace core

#endif // CORE_HPP
