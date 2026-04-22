//
// Created by Yeo Shu Heng on 22/4/26.
//

#ifndef SESSION_HPP
#define SESSION_HPP
#include <boost/asio/ip/tcp.hpp>
#include <sys/_types/_u_int64_t.h>

#include "include/command/parser.hpp"
#include "state.hpp"

namespace runtime {
struct DBSession {
    u_int64_t id;
    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
    std::atomic<SessionState> state{SessionState::ACTIVE};

    std::string read_buffer{};
    std::string write_buffer{};

    command::Parser parser{};

    DBSession(const u_int64_t id, std::shared_ptr<boost::asio::ip::tcp::socket> socket)
        : id(id), socket(std::move(socket)) {};
};
} // namespace runtime

#endif // SESSION_HPP
