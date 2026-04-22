//
// Created by Yeo Shu Heng on 13/4/26.
//

#include "../../include/command/command.hpp"

#include <charconv>
#include <format>
#include <stdexcept>

#include "../../include/commons/utils.hpp"

namespace command {

SetCommand::SetCommand(std::string key, core::stored_value value)
    : key(std::move(key)), value(std::move(value)), ttl_ms(0) {}

SetCommand::SetCommand(std::string key, core::stored_value value, uint32_t ttl_ms)
    : key(std::move(key)), value(std::move(value)), ttl_ms(ttl_ms) {}

Command build_command(ParsedResult& result) {
    auto& arr = result.value;

    if (arr.empty()) {
        throw std::runtime_error("empty command");
    }

    const std::string& cmd = arr[0];

    if (utils::fast_str_equals(cmd, "PING")) {
        return PingCommand{};
    }

    if (utils::fast_str_equals(cmd, "GET")) {
        if (arr.size() != 2)
            throw std::runtime_error("GET requires 1 argument");

        return GetCommand{arr[1]};
    }

    if (utils::fast_str_equals(cmd, "DEL")) {
        if (arr.size() != 2)
            throw std::runtime_error("DEL requires 1 argument");

        return DelCommand{arr[1]};
    }

    if (utils::fast_str_equals(cmd, "SET")) {
        if (arr.size() < 3)
            throw std::runtime_error("SET requires key + value");

        if (arr.size() > 4)
            throw std::runtime_error("SET only takes in 3 arguments, key, value and ttl");

        if (arr.size() == 3) {
            return SetCommand(arr[1], arr[2]);
        }

        uint32_t ttl_ms;

        auto [ptr, ec] = std::from_chars(arr[3].data(), arr[3].data() + arr[3].size(), ttl_ms);

        if (ec != std::errc{}) {
            throw std::runtime_error(std::format("failed to parse ttl, received: {}", arr[3]));
        }

        return SetCommand(arr[1], arr[2], ttl_ms);
    }

    throw std::runtime_error("unknown command: " + cmd);
}

} // namespace command