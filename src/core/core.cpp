//
// Created by Yeo Shu Heng on 14/4/26.
//

#include "../../include/core/core.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace core {
RedisCore::RedisCore(const size_t max_capacity, const wal_ptr& wal, const uint32_t ttl_budget)
    : lru_cache(max_capacity), max_capacity(max_capacity), wal(wal), ttl_budget(ttl_budget) {};

CoreResp RedisCore::execute(command::Command& cmd) {
    return std::visit(
        [this]<typename T>(const T& c) -> CoreResp {
            if constexpr (std::is_same_v<T, command::SetCommand>) {
                lru_cache.add(c.key, core::LRUObject(c.value), c.ttl_ms);
                return CoreResp{CoreResp::RespType::OK, std::nullopt, "OK"};
            } else if constexpr (std::is_same_v<T, command::DelCommand>) {
                if (const bool removed = lru_cache.remove(c.key); !removed) {
                    return CoreResp{CoreResp::RespType::ERROR, std::nullopt, "ERROR"};
                };
                return CoreResp{CoreResp::RespType::OK, std::nullopt, "OK"};
            } else if constexpr (std::is_same_v<T, command::GetCommand>) {
                const auto v = lru_cache.get(c.key);
                if (!v.has_value()) {
                    return CoreResp{CoreResp::RespType::NIL, std::nullopt, "NOT FOUND"};
                }
                return CoreResp{CoreResp::RespType::VALUE, v.value().val, "OK"};
            }
            return CoreResp{CoreResp::RespType::ERROR, std::nullopt, "UNRECOGNIZED COMMAND"};
        },
        cmd);
};

void RedisCore::evict() {
    lru_cache.remove_expired(ttl_budget);
};
} // namespace core
