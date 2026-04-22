//
// Created by Yeo Shu Heng on 22/4/26.
//

#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "./session.hpp"
#include "include/core/core.hpp"
#include "include/resp/serializer.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "include/disk/disk.hpp"
#include "state.hpp"

namespace runtime {
using io_ctx = boost::asio::io_context;
using acceptor = boost::asio::ip::tcp::acceptor;
using session_id = u_int64_t;
class DBEngine {
    io_ctx core_ctx;

    acceptor accept;
    std::unordered_map<session_id, std::shared_ptr<DBSession>> sessions;

    std::atomic<session_id> curr_id{0};

    std::unique_ptr<core::DBCore> core;
    std::unique_ptr<disk::DiskManager> disk_manager;

    std::atomic<EngineState> state{EngineState::STOPPED};
    response::Serializer serializer{};

    void accept_loop();
    void start_read(session_id id);
    void start_write(session_id id);
    void end_session(session_id id);

  public:
    DBEngine(const std::string& host, uint8_t port, std::unique_ptr<core::DBCore> core,
        std::unique_ptr<disk::DiskManager> disk_manager);
    void run();
    void close();
};
} // namespace runtime

#endif // ENGINE_HPP
