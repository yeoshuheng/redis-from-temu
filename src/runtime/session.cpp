//
// Created by Yeo Shu Heng on 18/4/26.
//
#include "../../include/runtime/session.hpp"

namespace runtime {
DBSession::DBSession(socket sock, const core_executor& core) : sock(std::move(sock)), core(core) {
    ip = sock.remote_endpoint().address().to_string();
    port = sock.remote_endpoint().port();
};

boost::asio::awaitable<void> DBSession::run() {
    try {
        for (;;) {
            const std::size_t n_bytes = co_await sock.async_read_some(
                boost::asio::buffer(read_buffer), boost::asio::use_awaitable);
            // stream current bytes into parser
            i_parser.feed(read_buffer.data(), n_bytes);
            while (true) {
                command::ParsedCommandOption cmd_opt = i_parser.next_msg();
                if (!cmd_opt.has_value())
                    break;
                command::Command cmd = cmd_opt.value();
                response::CoreResp resp = co_await core->execute(std::move(cmd));
                write_buffer.push(o_parser.serialize(resp));
            }
            co_await flush();
        }
    } catch (const boost::system::system_error& e) {
        if (e.code() == boost::asio::error::eof) {
            spdlog::info("session for ip={}, port={} closed", ip, port);
        } else if (e.code() == boost::asio::error::operation_aborted) {
            spdlog::info("session for ip={}, port={} aborted", ip, port);
        } else {
            spdlog::error(
                "problem handling session for ip={}, port={}, error_msg={}", ip, port, e.what());
        }
        close();
    }
    co_return;
};

void DBSession::close() {
    boost::system::error_code ec;
    sock.cancel(ec);
    sock.close(ec);
};

boost::asio::awaitable<void> DBSession::flush() {
    try {
        while (!write_buffer.empty()) {
            co_await boost::asio::async_write(
                sock, boost::asio::buffer(write_buffer.front()), boost::asio::use_awaitable);
            write_buffer.pop();
        }
    } catch (const boost::system::system_error& e) {
        if (e.code() != boost::asio::error::operation_aborted) {
            co_return;
        }
        spdlog::error(
            "problem writing resp for session ip={}, port={}, error_msg={}", ip, port, e.what());
        throw;
    }
};
} // namespace runtime