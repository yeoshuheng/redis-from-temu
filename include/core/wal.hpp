//
// Created by Yeo Shu Heng on 15/4/26.
//

#ifndef WAL_HPP
#define WAL_HPP

#include "./wal_codec.hpp"
#include "include/command/command.hpp"
#include <functional>
#include <string>

namespace core {
constexpr size_t RAM_BUFFER_BYTES = 64 * 1024;
class WAL {
  private:
    std::mutex m;
    std::string path;
    WALCodec codec{};
    uint8_t fd;
    std::FILE* file;

  public:
    explicit WAL(const std::string& path);
    void append(const command::Command& cmd); // for core thread, cheap writes to RAM
    void flush();                             // for disk thread, expensive fsync
    void recover(std::function<void(command::Command)> const& replay);
    void clear();
};
} // namespace core

#endif // WAL_HPP
