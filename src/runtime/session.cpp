//
// Created by Yeo Shu Heng on 22/4/26.
//
#include "../../include/runtime/session.hpp"

namespace runtime {
DBSession::DBSession(boost::asio::ip::tcp::socket sock, core::DBCore& core)
    : sock(std::move(sock)), core(core) {}

boost::asio::awaitable<void> DBSession::do_read() {
    boost::system::error_code ec;
    std::array<char, 4096> buffer{};
    while (true) {
        const size_t n_bytes = co_await sock.async_read_some(boost::asio::buffer(buffer),
            boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        if (ec) {
            if (ec != boost::asio::error::operation_aborted && ec != boost::asio::error::eof &&
                ec != boost::asio::error::connection_reset) {
                spdlog::warn("read error: {}", ec.message());
            }
            co_return;
        }
        parser.feed(buffer.data(), n_bytes);
        while (parser.has_next_msg()) {
            const auto cmd_opt = parser.next_msg();
            if (!cmd_opt.has_value()) {
                break;
            }
            auto cmd = cmd_opt.value();
            const auto resp = core.execute(cmd);
            co_await do_write(response::Serializer::serialize(resp));
        }
    }
};

boost::asio::awaitable<void> DBSession::do_write(std::string payload) {
    boost::system::error_code ec;
    co_await boost::asio::async_write(sock, boost::asio::buffer(payload),
        boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    if (ec) {
        spdlog::error("write error: {}", ec.message());
        sock.close();
    }
};

boost::asio::awaitable<void> DBSession::run() {
    spdlog::info("starting new session for connection, addr={}",
        sock.remote_endpoint().address().to_string());
    co_await do_read();
};
} // namespace runtime