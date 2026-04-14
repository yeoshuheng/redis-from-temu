//
// Created by Yeo Shu Heng on 13/4/26.
//

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <charconv>
#include <format>
#include <string>
#include <utility>
#include <vector>

#include "../commons/utils.hpp"
#include "../storage/object.hpp"
#include "parsed_results.hpp"

namespace command {

struct PingCommand {};

struct SetCommand {
    std::string key;
    storage::stored_value value;
    uint32_t ttl_ms;

    SetCommand(std::string key, storage::stored_value value)
        : key(std::move(key)), value(std::move(value)), ttl_ms(0) {};
    SetCommand(std::string key, storage::stored_value value, const uint32_t ttl_ms)
        : key(std::move(key)), value(std::move(value)), ttl_ms(ttl_ms) {};
};

struct GetCommand {
    std::string key;
};

struct DelCommand {
    std::string key;
};

using Command = std::variant<PingCommand, SetCommand, GetCommand, DelCommand>;

inline Command build_command(ParsedResult& result) {
    auto& arr = result.value;
    if (arr.empty()) {
        throw std::runtime_error("empty command");
    }
    const std::string& cmd = arr[0];
    if (utils::fast_str_equals(cmd, "PING")) {
        return PingCommand{};
    }
    if (utils::fast_str_equals(cmd, "GET")) {
        if (arr.size() != 2) throw std::runtime_error("GET requires 1 argument");

        return GetCommand{arr[1]};
    }
    if (utils::fast_str_equals(cmd, "DEL")) {
        if (arr.size() != 2) throw std::runtime_error("DEL requires 1 argument");

        return DelCommand{arr[1]};
    }
    if (utils::fast_str_equals(cmd, "SET")) {
        if (arr.size() < 3) throw std::runtime_error("SET requires key + value");
        if (arr.size() > 4)
            throw std::runtime_error("SET only takes in 3 arguments, key, value and ttl");
        if (arr.size() == 3) {
            return SetCommand(arr[1], arr[2]);
        } else {
            uint32_t ttl_ms;
            if (auto [ptr, ec] =
                    std::from_chars(arr[3].data(), arr[3].data() + arr[3].size(), ttl_ms);
                ec != std::errc{}) {
                throw std::runtime_error(std::format("failed to parse ttl, received: {}", arr[3]));
            }
            return SetCommand(arr[1], arr[2], ttl_ms);
        }
    }
    throw std::runtime_error("unknown command: " + cmd);
};

template <typename T>
const T& as(const Command& cmd) {
    return std::get<T>(cmd);
}

template <typename T>
const T& as_value(const storage::stored_value& v) {
    return std::get<T>(v);
}

}  // namespace command

#endif  // COMMAND_HPP
