//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef CORE_HPP
#define CORE_HPP

#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "../command/parser.hpp"
#include "../storage/lru_cache.hpp"
#include "../storage/object.hpp"

namespace core {
class RedisCore {
    using channel = boost::lockfree::spsc_queue<command::Command>;
    using io_ctx = boost::asio::io_context;
    using timer = boost::asio::steady_timer;

   private:
    storage::LRUCache<std::string, storage::StoredObject> lru_cache;
    size_t max_capacity;
    std::shared_ptr<channel> input;
    std::shared_ptr<channel> output;
    std::atomic<bool> is_running{false};

    io_ctx& ctx;
    uint32_t poll_interval;
    uint32_t ttl_interval;
    uint32_t ttl_budget;
    timer poll_timer;
    timer ttl_timer;

    boost::asio::awaitable<void> poll_loop();
    boost::asio::awaitable<void> ttl_loop();
    void execute(command::Command& cmd);

   public:
    explicit RedisCore(io_ctx& ctx, size_t max_capacity, const std::shared_ptr<channel>& in_channel,
                       const std::shared_ptr<channel>& out_channel, uint32_t poll_interval_ms,
                       uint32_t ttl_interval_ms, uint32_t ttl_budget);
    void start();
    void shutdown();
};
}  // namespace core

#endif  // CORE_HPP
