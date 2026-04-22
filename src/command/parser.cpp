//
// Created by Yeo Shu Heng on 13/4/26.
//

#include "../../include/command/parser.hpp"

#include <spdlog/spdlog.h>

namespace command {
constexpr char BULK_STR_START = '$';
constexpr char ARRAY_START = '*';
constexpr size_t BUFFER_CLEANUP_LIMIT = 1024;

void Parser::feed(const char* data, const size_t len) {
    buffer.insert(buffer.end(), data, data + len);
    cache_empty = false;
    cached_result = {};
}

bool Parser::has_next_msg() {
    if (!cache_empty) {
        return true;
    }
    const ResultOption parsed_result = parse_array(offset);
    if (!parsed_result.has_value()) {
        return false;
    }
    cache_empty = false;
    cached_result = parsed_result.value();
    return true;
}

ParsedCommandOption Parser::next_msg() {
    spdlog::debug("buffer: {} offset: {}", std::string(buffer.begin(), buffer.end()), offset);
    ParsedResult result;

    if (cache_empty) {
        const ResultOption parsed_opt = parse_array(offset);
        if (!parsed_opt.has_value()) {
            return std::nullopt;
        }
        result = parsed_opt.value();
        spdlog::debug("parsed result: consumed={}, size={}", result.consumed, result.value.size());
    } else {
        result = cached_result;
    }

    if (result.consumed == 0 || offset + result.consumed > buffer.size()) {
        return std::nullopt;
    }
    offset += result.consumed;

    if (offset >= BUFFER_CLEANUP_LIMIT) {
        buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(offset));
        offset = 0;
    }
    spdlog::debug("offset updated: {}", offset);

    return build_command(result);
}

ResultOption Parser::parse_array(const size_t start) {
    // RESP2 array: *3\r\n$3\r\n{cmd}\r\n$3\r\n{key}\r\n$5\r\n{value}\r\n
    // where cmd, key and value are bulk strings.
    if (start >= buffer.size() || buffer[start] != ARRAY_START) {
        return std::nullopt;
    }
    const std::optional<size_t> terminal_idx_opt = find_terminal_idx(start);
    if (!terminal_idx_opt.has_value()) {
        return std::nullopt;
    }
    const long crlf_idx = static_cast<long>(terminal_idx_opt.value());
    const long start_long = static_cast<long>(start);

    // array header in this case is *3\r\n
    const size_t array_len =
        std::stoi(std::string(buffer.begin() + start_long + 1, buffer.begin() + crlf_idx));
    spdlog::debug("array_len: {}", array_len);

    size_t curr_position = crlf_idx + 1;
    std::vector<std::string> parsed_array;
    for (size_t i = 0; i < array_len;
        i++) { // RESP2 arrays terminate based on elements parsed, not CRLF
        if (curr_position > buffer.size()) {
            return std::nullopt;
        }
        ResultOption bulk_str = parse_bulk(curr_position);
        if (!bulk_str.has_value()) {
            return std::nullopt;
        }
        spdlog::debug("bulk_str parsed: {}", bulk_str.value().value[0]);
        parsed_array.push_back(bulk_str.value().value[0]);
        curr_position += bulk_str.value().consumed;
    }
    return ParsedResult{curr_position - start, parsed_array};
}

ResultOption Parser::parse_bulk(const size_t start) {
    // RESP2 bulk string: $2\r\nhi\r\n
    if (start >= buffer.size() || buffer[start] != BULK_STR_START) {
        return std::nullopt;
    }
    const std::optional<size_t> terminal_idx_opt = find_terminal_idx(start);
    if (!terminal_idx_opt.has_value()) {
        return std::nullopt;
    }
    const long crlf_idx = static_cast<long>(terminal_idx_opt.value());
    const long start_long = static_cast<long>(start);
    const size_t data_len =
        std::stoi(std::string(buffer.begin() + start_long + 1, buffer.begin() + crlf_idx));

    // header is: $2\r\n
    const size_t header_len = (crlf_idx - start) + 1;

    // the whole bulk string to the second CRLF.
    const size_t bulk_str_len = data_len + header_len + 2;

    spdlog::debug("start: {}, buffer_size: {}, header_len: {}, data_len: {}, bulk_str_len: {}",
        start, buffer.size(), header_len, data_len, bulk_str_len);
    if (start + bulk_str_len > buffer.size()) {
        return std::nullopt;
    }
    const auto common_start = buffer.begin() + start_long + static_cast<long>(header_len);
    const std::string data(common_start, common_start + static_cast<long>(data_len));
    return ParsedResult{bulk_str_len, {data}};
}

std::optional<size_t> Parser::find_terminal_idx(const size_t start) const {
    // in RESP2, carriage return (\r) and line feed (\n) together represents EOC.
    for (size_t i = start + 1; i < buffer.size(); ++i) {
        if (buffer[i] == '\n' && buffer[i - 1] == '\r') {
            return i;
        }
    }
    return std::nullopt;
}

} // namespace command
