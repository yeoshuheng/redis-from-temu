//
// Created by Yeo Shu Heng on 13/4/26.
//

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include <vector>

#include "../commons/utils.hpp"
#include "parsed_results.hpp"

namespace command {
enum class CommandType { PING, SET, GET, DEL };

struct Command {
    CommandType type;
    std::vector<std::string> args;
};

inline Command build_command(ParsedResult& result) {
    auto& arr = result.value;
    if (arr.empty()) {
        throw std::runtime_error("Empty command");
    }

    const std::string& cmd = arr[0];
    CommandType type;

    if (utils::fast_str_equals(cmd, "PING")) {
        type = CommandType::PING;
        return Command{type, {}};
    }

    if (utils::fast_str_equals(cmd, "SET")) {
        type = CommandType::SET;
    } else if (utils::fast_str_equals(cmd, "GET")) {
        type = CommandType::GET;
    } else if (utils::fast_str_equals(cmd, "DEL")) {
        type = CommandType::DEL;
    } else {
        throw std::runtime_error("unknown command: " + cmd);
    }

    std::vector<std::string> args;
    args.reserve(arr.size() - 1);
    for (size_t i = 1; i < arr.size(); ++i) {
        args.push_back(std::move(arr[i]));
    }
    return Command{type, std::move(args)};
};
}  // namespace command

#endif  // COMMAND_HPP
