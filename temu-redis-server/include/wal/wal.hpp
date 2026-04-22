//
// Created by Yeo Shu Heng on 15/4/26.
//

#ifndef WAL_HPP
#define WAL_HPP

#include "../wal/wal_codec.hpp"
#include "../../include/command/command.hpp"
#include <functional>
#include <string>

namespace wal {
constexpr size_t RAM_BUFFER_BYTES = 64 * 1024;
class WAL {
  private:
    std::mutex m;
    std::string path;
    WALCodec codec{};
    int fd;
    std::FILE* file;
    std::atomic<uint64_t> n_bytes_written;
    std::atomic<uint64_t> n_bytes_flushed;

  public:
    explicit WAL(const std::string& path);
    ~WAL();
    void append(const command::Command& cmd); // for core thread, cheap writes to RAM
    void flush();                             // for disk thread, expensive fsync
    void recover(std::function<void(command::Command)> const& replay);
    void clear();
    bool is_empty() const;
};
} // namespace wal

#endif // WAL_HPP
