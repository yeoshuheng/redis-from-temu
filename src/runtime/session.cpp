//
// Created by Yeo Shu Heng on 18/4/26.
//
#include "../../include/runtime/session.hpp"

namespace runtime {
DBSession::DBSession(socket sock, const core_executor& core) : sock(std::move(sock)), core(core) {
    ip = sock.remote_endpoint().address().to_string();
    port = sock.remote_endpoint().port();
};

DBSession::~DBSession() {
    close();
}

boost::asio::awaitable<void> DBSession::run() {
    try {
        while (true) {
            const std::size_t n_bytes = co_await sock.async_read_some(
                boost::asio::buffer(read_buffer), boost::asio::use_awaitable);
            // stream current bytes into parser
            i_parser.feed(read_buffer.data(), n_bytes);
            while (true) {
                command::ParsedCommandOption cmd_opt = i_parser.next_msg();
                if (!cmd_opt.has_value()) {
                    break;
                }
                command::Command cmd = cmd_opt.value();
                response::CoreResp resp = co_await core->execute(cmd);
                write_buffer.push(o_parser.serialize(resp));
            }
            co_await flush();
        }
    } catch (std::exception& e) {
        spdlog::error(
            "problem handling session for ip={}, port={}, error_msg={}", ip, port, e.what());
        close();
    }
};

void DBSession::close() {
    sock.close();
};

boost::asio::awaitable<void> DBSession::flush() {
    while (!write_buffer.empty()) {
        co_await boost::asio::async_write(
            sock, boost::asio::buffer(write_buffer.front()), boost::asio::use_awaitable);
        write_buffer.pop();
    }
};
} // namespace runtime