//
// Created by Yeo Shu Heng on 22/4/26.
//

#ifndef SESSION_HPP
#define SESSION_HPP
#include <boost/asio/ip/tcp.hpp>
#include <memory>

#include "../core/core.hpp"
#include "../resp/serializer.hpp"

namespace runtime {
class DBSession : std::enable_shared_from_this<DBSession> {
    boost::asio::ip::tcp::socket sock;
    core::DBCore& core;

    command::Parser parser{};
    response::Serializer serializer{};

    boost::asio::awaitable<void> do_read();
    boost::asio::awaitable<void> do_write(std::string payload);

  public:
    explicit DBSession(boost::asio::ip::tcp::socket sock, core::DBCore& core);
    boost::asio::awaitable<void> run();
};
} // namespace runtime

#endif // SESSION_HPP
