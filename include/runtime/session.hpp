//
// Created by Yeo Shu Heng on 18/4/26.
//

#ifndef SESSION_HPP
#define SESSION_HPP
#include "../command/parser.hpp"
#include "../core/core.hpp"
#include "../resp/serializer.hpp"
#include "executor.hpp"

namespace runtime {
using core_executor = std::shared_ptr<DBCoreExecutor>;
using input_parser = command::Parser;
using socket = boost::asio::ip::tcp::socket;

class DBSession final {
    std::string ip;
    std::string port;
    socket sock;
    core_executor core;
    input_parser i_parser{};
    response::Serializer o_parser{};
    std::array<char, 4096> read_buffer{};
    std::queue<std::string> write_buffer{};
    boost::asio::awaitable<void> flush();

  public:
    DBSession(socket sock, const core_executor& core);
    boost::asio::awaitable<void> run();
    void close();
};
} // namespace runtime

#endif // SESSION_HPP
