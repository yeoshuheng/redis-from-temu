//
// Created by Yeo Shu Heng on 15/4/26.
//

#ifndef WAL_CODEC_HPP
#define WAL_CODEC_HPP

#include <string>

#include "../../include/command/command.hpp"

namespace wal {
using core::stored_value;
enum class WALCommand { PING = 0, SET = 1, GET = 2, DEL = 3 };
enum class WALValue { STRING = 0, INT64 = 1, FLOAT = 2, DOUBLE = 3 };
class WALCodec {
    static void ensure(const char* ptr, const char* end, size_t n);

  public:
    static std::string serialize(const command::Command& cmd);
    static command::Command deserialize(const std::string& str);
    static std::string serialize_stored_value(const stored_value& val);
    static stored_value deserialize_stored_value(const char*& str, const char* end);
};
} // namespace wal

#endif // WAL_CODEC_HPP
