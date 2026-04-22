//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef CORE_HPP
#define CORE_HPP

#include <boost/asio.hpp>

#include "../command/parser.hpp"
#include "../resp/resp.hpp"
#include "../wal/wal.hpp"
#include "lru_cache.hpp"
#include "object.hpp"

namespace core {
using DBCache = LRUCache<std::string, core::LRUObject>;
using cache_ptr = std::unique_ptr<DBCache>;
using wal_ptr = std::shared_ptr<wal::WAL>;
using response::CoreResp;
class DBCore final {
  private:
    cache_ptr lru_cache;
    size_t max_capacity;
    uint32_t ttl_budget;
    wal_ptr wal;

    [[nodiscard]] bool should_persist(const command::Command& cmd) const;
    void persist(const command::Command& cmd) const;

  public:
    DBCore(size_t max_capacity, const wal_ptr& wal, uint32_t ttl_budget);

    CoreResp execute(command::Command& cmd);
    void evict() const;
};
} // namespace core

#endif // CORE_HPP
