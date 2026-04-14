//
// Created by Yeo Shu Heng on 13/4/26.
//

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <utility>
#include <variant>

#include "../storage/object.hpp"
#include "parsed_results.hpp"

namespace command {

struct PingCommand {};

struct SetCommand {
    std::string key;
    storage::stored_value value;
    uint32_t ttl_ms;

    SetCommand(std::string key, storage::stored_value value);
    SetCommand(std::string key, storage::stored_value value, uint32_t ttl_ms);
};

struct GetCommand {
    std::string key;
};

struct DelCommand {
    std::string key;
};

using Command = std::variant<PingCommand, SetCommand, GetCommand, DelCommand>;

Command build_command(ParsedResult& result);

template <typename T> const T& as(const Command& cmd) {
    return std::get<T>(cmd);
}

template <typename T> const T& as_value(const storage::stored_value& v) {
    return std::get<T>(v);
}

} // namespace command

#endif // COMMAND_HPP