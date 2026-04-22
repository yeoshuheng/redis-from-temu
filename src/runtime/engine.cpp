//
// Created by Yeo Shu Heng on 22/4/26.
//
#include <utility>

#include "../../include/runtime/engine.hpp"
#include "../../include/runtime/session.hpp"

namespace runtime {
boost::asio::awaitable<void> DBEngine::accept_loop() {
    while (state.load(std::memory_order_acquire) == EngineState::RUNNING) {
        boost::system::error_code ec;
        boost::asio::ip::tcp::socket sock(ctx);
        co_await accept.async_accept(
            sock, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            if (ec == boost::asio::error::operation_aborted ||
                ec == boost::asio::error::bad_descriptor) {
                co_return;
            }
            spdlog::error("error in accepting connection, {}", ec.message());
            co_return;
        }
        auto session = std::make_shared<DBSession>(std::move(sock), *core);
        boost::asio::co_spawn(
            ctx,
            [sess = std::move(session)]() mutable -> boost::asio::awaitable<void> {
                co_await sess->run();
            },
            boost::asio::detached);
    }
};

boost::asio::awaitable<void> DBEngine::term_loop() {
    boost::asio::signal_set signals(ctx, SIGTERM, SIGINT);
    boost::system::error_code ec;
    const int signum =
        co_await signals.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if (ec) {
        if (ec != boost::asio::error::operation_aborted) {
            spdlog::error("error whilst waiting for term signal, {}", ec.message());
        }
        co_return;
    }
    spdlog::info("term signal received, signum={}", signum);
    stop();
};

boost::asio::awaitable<void> DBEngine::disk_manager_loop() {
    while (state.load(std::memory_order::acquire) == EngineState::RUNNING) {
        boost::system::error_code ec;
        co_await disk_check_timer.async_wait(
            boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            co_return;
        }
        const uint64_t last_hb = hb_state->disk_heartbeat.load(std::memory_order_relaxed);
        const uint64_t now = disk::Clock::now().time_since_epoch().count();
        const uint64_t threshold =
            static_cast<uint64_t>(cfg.disk_state_threshold_ms) * 1'000'000ULL;
        ;
        if (last_hb > 0 && now - last_hb > threshold) {
            spdlog::error("disk manager missed heartbeat, restarting");
            disk->shutdown();
            disk->start();
        }
    }
};

DBEngine::DBEngine(EngineConfig  config)
    : cfg(std::move(config)),
      accept(ctx, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), cfg.port)),
      disk_check_timer(ctx) {
    accept.set_option(boost::asio::socket_base::reuse_address(true));
    auto wal = std::make_shared<wal::WAL>(cfg.wal_path);
    core = std::make_shared<core::DBCore>(cfg.db_max_capacity, wal, cfg.ttl_budget);
    hb_state = std::make_shared<commons::ThreadHeartBeatState>();
    disk = std::make_shared<disk::DiskManager>(wal, hb_state, cfg.disk_flush_interval_ms);
};

DBEngine::~DBEngine() {
    stop();
};

void DBEngine::stop() {
    if (state.exchange(EngineState::STOPPED, std::memory_order_acquire) == EngineState::STOPPED) {
        return;
    }
    spdlog::info("stopping engine...");
    boost::system::error_code ec;
    accept.close(ec);
    group.stop();
    spdlog::info("engine stopped");
};

void DBEngine::start() {
    if (state.exchange(EngineState::RUNNING, std::memory_order_acquire) == EngineState::RUNNING) {
        return;
    }
    spdlog::info("starting engine...");
    disk->start();
    group.spawn(ctx, [this] { return accept_loop(); });
    group.spawn(ctx, [this] { return term_loop(); });
    group.spawn(ctx, [this] { return disk_manager_loop(); });
    spdlog::info("engine started...");
    ctx.run();
    group.join();
    disk->shutdown();
};
} // namespace runtime