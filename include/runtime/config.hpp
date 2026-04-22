//
// Created by Yeo Shu Heng on 22/4/26.
//

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>
#include <string>

namespace runtime {
struct EngineConfig {
    uint16_t port = 6379;
    size_t db_max_capacity = 1024;
    uint32_t ttl_budget = 100;
    uint32_t disk_flush_interval_ms = 500;
    uint32_t disk_state_threshold_ms = 500;
    std::string wal_path = "wal.log";
};
} // namespace runtime

#endif // CONFIG_HPP
