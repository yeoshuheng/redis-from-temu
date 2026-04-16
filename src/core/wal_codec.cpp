//
// Created by Yeo Shu Heng on 16/4/26.
//
#include "../../include/core/wal_codec.hpp"
#include "../../include/commons/utils.hpp"
#include "spdlog/spdlog.h"

namespace core {
std::string WALCodec::serialize_stored_value(const stored_value& val) {
    std::string serialized;
    std::visit(
        [&]<typename T>(T&& v) {
            using U = std::decay_t<T>;
            if constexpr (std::is_same_v<U, int64_t>) {
                serialized.push_back(static_cast<char>(WALValue::INT64));
                utils::write_u32(serialized, sizeof(v));
                serialized.append(reinterpret_cast<const char*>(&v), sizeof(v));
            } else if constexpr (std::is_same_v<U, double>) {
                serialized.push_back(static_cast<char>(WALValue::DOUBLE));
                utils::write_u32(serialized, sizeof(v));
                serialized.append(reinterpret_cast<const char*>(&v), sizeof(v));
            } else if constexpr (std::is_same_v<U, float>) {
                serialized.push_back(static_cast<char>(WALValue::FLOAT));
                utils::write_u32(serialized, sizeof(v));
                serialized.append(reinterpret_cast<const char*>(&v), sizeof(v));
            } else if constexpr (std::is_same_v<U, std::string>) {
                serialized.push_back(static_cast<char>(WALValue::STRING));
                utils::write_u32(serialized, v.size());
                serialized.append(v);
            }
        },
        val);
    return serialized;
};

core::stored_value WALCodec::deserialize_stored_value(const char*& str) {
    const uint8_t type = *str++;
    const uint32_t size = utils::read_u32(str);
    switch (static_cast<WALValue>(type)) {
    case WALValue::INT64: {
        int64_t v;
        std::memcpy(&v, str, sizeof(v));
        str += sizeof(v);
        return v;
    }
    case WALValue::DOUBLE: {
        double v;
        std::memcpy(&v, str, sizeof(v));
        str += sizeof(v);
        return v;
    }
    case WALValue::FLOAT: {
        float v;
        std::memcpy(&v, str, sizeof(v));
        str += sizeof(v);
        return v;
    }
    case WALValue::STRING: {
        std::string v(str, size);
        str += size;
        return v;
    }
    }
    throw std::runtime_error("unknown stored value, WAL corrupted, stored value can only be "
                             "string, float, double or int");
};

std::string WALCodec::serialize(const command::Command& cmd) {
    std::string log;
    uint8_t type = 0;
    std::visit(
        [&]<typename T>(T&& c) {
            using U = std::decay_t<T>;
            if constexpr (std::is_same_v<U, command::PingCommand>) {
                type = static_cast<uint8_t>(WALCommand::PING);
            } else if constexpr (std::is_same_v<U, command::SetCommand>) {
                type = static_cast<uint8_t>(WALCommand::SET);
                utils::write_u32(log, c.key.size());
                log.append(c.key);
                const std::string serialized_value = serialize_stored_value(c.value);
                log.append(serialized_value);
                utils::write_u32(log, c.ttl_ms);
            } else if constexpr (std::is_same_v<U, command::DelCommand>) {
                type = static_cast<uint8_t>(WALCommand::DEL);
                utils::write_u32(log, c.key.size());
                log.append(c.key);
            } else if constexpr (std::is_same_v<U, command::GetCommand>) {
                type = static_cast<uint8_t>(WALCommand::GET);
                utils::write_u32(log, c.key.size());
                log.append(c.key);
            }
        },
        cmd);
    std::string out;
    out.push_back(static_cast<char>(type));
    utils::write_u32(out, log.size());
    out += log;
    return out;
}

command::Command WALCodec::deserialize(const std::string& str) {
    const char* begin = str.data();
    const char* ptr = begin;
    uint8_t type = *ptr++; // first byte is type
    uint32_t args_size = utils::read_u32(ptr);
    const char* end = ptr + args_size;

    auto check = [&](const char* p) {
        if (p > end)
            throw std::runtime_error("WAL data corrupted, data is out-of-bound");
    };

    spdlog::debug("recv: {}, type: {}, arg_size: {}", str, type, args_size);
    switch (static_cast<WALCommand>(type)) {
    case WALCommand::PING:
        return command::PingCommand{};
    case WALCommand::SET: {
        const uint32_t key_len = utils::read_u32(ptr);
        check(ptr);
        std::string key(ptr, key_len);
        ptr += key_len;
        check(ptr);

        const stored_value deserialized_value = deserialize_stored_value(ptr);
        check(ptr);

        const uint32_t ttl = utils::read_u32(ptr);
        check(ptr);

        return command::SetCommand{std::move(key), deserialized_value, ttl};
    }
    case WALCommand::DEL: {
        const uint32_t dkey_len = utils::read_u32(ptr);
        check(ptr);
        std::string dkey(ptr, dkey_len);
        ptr += dkey_len;
        return command::DelCommand{std::move(dkey)};
    }
    case WALCommand::GET: {
        const uint32_t gkey_len = utils::read_u32(ptr);
        check(ptr);
        std::string gkey(ptr, gkey_len);
        ptr += gkey_len;
        return command::GetCommand{std::move(gkey)};
    }
    }
    throw std::runtime_error("unknown command type, WAL corrupted, cannot read from WAL");
};
} // namespace core