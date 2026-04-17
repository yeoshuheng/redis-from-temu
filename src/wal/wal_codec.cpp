//
// Created by Yeo Shu Heng on 16/4/26.
//
#include "../../include/wal/wal_codec.hpp"
#include "../../include/commons/utils.hpp"
#include "spdlog/spdlog.h"

namespace wal {
void WALCodec::ensure(const char* ptr, const char* end, const size_t n) {
    if (ptr + n > end) {
        throw std::runtime_error("command data is corrupted, WAL is reading out of bounds");
    }
};

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

core::stored_value WALCodec::deserialize_stored_value(const char*& str, const char* end) {
    ensure(str, end, 1);
    const uint8_t type = *str++;
    ensure(str, end, sizeof(uint32_t));
    const uint32_t size = utils::read_u32(str);
    switch (static_cast<WALValue>(type)) {
    case WALValue::INT64: {
        ensure(str, end, sizeof(int64_t));
        int64_t v;
        std::memcpy(&v, str, sizeof(v));
        str += sizeof(v);
        return v;
    }
    case WALValue::DOUBLE: {
        ensure(str, end, sizeof(double));
        double v;
        std::memcpy(&v, str, sizeof(v));
        str += sizeof(v);
        return v;
    }
    case WALValue::FLOAT: {
        ensure(str, end, sizeof(float));
        float v;
        std::memcpy(&v, str, sizeof(v));
        str += sizeof(v);
        return v;
    }
    case WALValue::STRING: {
        ensure(str, end, size);
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
    const char* ptr = str.data();
    const char* end = ptr + str.size();
    ensure(ptr, end, 1);
    uint8_t type = *ptr++; // first byte is type

    ensure(ptr, end, sizeof(uint32_t));
    uint32_t args_size = utils::read_u32(ptr);
    const char* args_end = ptr + args_size;

    spdlog::debug("recv: {}, type: {}, arg_size: {}", str, type, args_size);
    switch (static_cast<WALCommand>(type)) {
    case WALCommand::PING:
        return command::PingCommand{};
    case WALCommand::SET: {
        ensure(ptr, args_end, sizeof(uint32_t));
        const uint32_t key_len = utils::read_u32(ptr);

        ensure(ptr, args_end, key_len);
        std::string key(ptr, key_len);
        ptr += key_len;

        const stored_value deserialized_value = deserialize_stored_value(ptr, args_end);

        ensure(ptr, args_end, sizeof(uint32_t));
        const uint32_t ttl = utils::read_u32(ptr);

        return command::SetCommand{std::move(key), deserialized_value, ttl};
    }
    case WALCommand::DEL: {
        ensure(ptr, args_end, sizeof(uint32_t));
        const uint32_t dkey_len = utils::read_u32(ptr);

        ensure(ptr, args_end, dkey_len);
        std::string dkey(ptr, dkey_len);
        ptr += dkey_len;
        return command::DelCommand{std::move(dkey)};
    }
    case WALCommand::GET: {
        ensure(ptr, args_end, sizeof(uint32_t));
        const uint32_t gkey_len = utils::read_u32(ptr);

        ensure(ptr, args_end, gkey_len);
        std::string gkey(ptr, gkey_len);
        ptr += gkey_len;
        return command::GetCommand{std::move(gkey)};
    }
    }
    throw std::runtime_error("unknown command type, WAL corrupted, cannot read from WAL");
};
} // namespace wal