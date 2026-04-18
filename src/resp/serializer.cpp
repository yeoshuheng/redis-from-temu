//
// Created by Yeo Shu Heng on 18/4/26.
//
#include "../../include/resp/serializer.hpp"

#include <format>

namespace response {
std::string Serializer::serialize_value(const stored_value& value) {
    return std::visit(
        []<typename T>(const T& v) -> std::string {
            if constexpr (std::is_same_v<T, std::string>) {
                return v;
            } else if constexpr (std::is_same_v<T, double>) {
                return std::to_string(v);
            } else if constexpr (std::is_same_v<T, float>) {
                return std::to_string(v);
            } else if constexpr (std::is_same_v<T, int64_t>) {
                return std::to_string(v);
            } else {
                static_assert(!sizeof(T), "invalid type in stored_value");
            }
        },
        value);
};

std::string Serializer::serialize(const CoreResp& resp) {
    std::string val;
    switch (resp.type) {
    case CoreResp::RespType::OK:
        return "+OK\r\n";
    case CoreResp::RespType::ERROR:
        return std::format("-{}\r\n", resp.message);
    case CoreResp::RespType::VALUE:
        if (!resp.value.has_value()) {
            return "$-1\r\n";
        }
        val = serialize_value(resp.value.value());
        return std::format("${}\r\n{}\r\n", val.size(), val);
    case CoreResp::RespType::NIL:
        return "$-1\r\n";
    }
    throw std::runtime_error("unknown core response type");
};
} // namespace response