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
using wal_ptr = std::shared_ptr<wal::WAL>;
class RedisCore final {
  private:
    RedisCache lru_cache;
    size_t max_capacity;
    uint32_t ttl_budget;
    wal_ptr wal;

    bool should_persist(const command::Command& cmd) const;
    void persist(const command::Command& cmd) const;

  public:
    RedisCore(size_t max_capacity, const wal_ptr& wal, uint32_t ttl_budget);

    CoreResp execute(command::Command& cmd);
    void evict();
};
} // namespace core

#endif // CORE_HPP
