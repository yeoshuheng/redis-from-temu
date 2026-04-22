//
// Created by Yeo Shu Heng on 22/4/26.
//

#include "../include/runtime/engine.hpp"
#include <CLI/CLI.hpp>

int main(const int argc, char* argv[]) {
    CLI::App app{"redis-from-temu, redis but shittier"};
    runtime::EngineConfig cfg;
    app.add_option("--port", cfg.port, "the port to listen on")->default_val(6379);
    app.add_option("--wal-path", cfg.wal_path, "wal file path")->default_val("wal.log");
    app.add_option("--capacity", cfg.db_max_capacity, "maximum capacity before LRU eviction")
        ->default_val(1024);
    app.add_option("--flush-interval", cfg.disk_flush_interval_ms,
           "interval to flush cache data to disk (ms)")
        ->default_val(500);
    app.add_option("--ttl-budget", cfg.ttl_budget, "how many nodes we can actively evict")
        ->default_val(100);
    CLI11_PARSE(app, argc, argv);
    runtime::DBEngine engine(cfg);
    engine.start();
    return 0;
}