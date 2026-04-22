//
// Created by Yeo Shu Heng on 22/4/26.
//

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "config.hpp"
#include "include/core/core.hpp"
#include "include/disk/disk.hpp"
#include "state.hpp"

namespace runtime {
using io_context = boost::asio::io_context;
using acceptor = boost::asio::ip::tcp::acceptor;
using timer = boost::asio::steady_timer;
class DBEngine {
    commons::CoroutineGroup group{};

    EngineConfig cfg;

    io_context ctx;
    acceptor accept;

    std::atomic<EngineState> state{EngineState::STOPPED};

    std::shared_ptr<core::DBCore> core;
    std::shared_ptr<disk::DiskManager> disk;

    std::shared_ptr<commons::ThreadHeartBeatState> hb_state;
    timer disk_check_timer;

    boost::asio::awaitable<void> disk_manager_loop();
    boost::asio::awaitable<void> accept_loop();
    boost::asio::awaitable<void> term_loop();
    void stop();

  public:
    explicit DBEngine(const EngineConfig& config);
    ~DBEngine();
    void start();
};
} // namespace runtime

#endif // ENGINE_HPP
