//
// Created by Yeo Shu Heng on 16/4/26.
//
#include "../../include/core/wal.hpp"

#include "spdlog/spdlog.h"
#include <boost/asio/detail/mutex.hpp>
#include <zlib.h>

namespace core {
WAL::WAL(const std::string &path): path(path) {
    // https://en.cppreference.com/w/cpp/io/c/fopen.html
    // a+b means open file in append r/w + binary
    file = fopen(path.c_str(), "a+b");
    if (!file) throw std::runtime_error(std::format("failed to open WAL file at {}", path));
    setvbuf(file, nullptr, _IOFBF, RAM_BUFFER_BYTES);
    fd = fileno(file);
};

void WAL::append(const command::Command& cmd) {
    const std::string serialized = codec.serialize(cmd);
    const uint32_t size = serialized.size();
    // create checksum
    const uint32_t crc = crc32(0, reinterpret_cast<const Bytef*>(serialized.data()), size);
    std::lock_guard<std::mutex> lock(m);
    if (fwrite(&size, sizeof(size), 1, file) != 1) {
        throw std::runtime_error(std::format("failed to write size to WAL file at {}", path));
    };
    if (fwrite(&crc, sizeof(crc), 1, file) != 1) {
        throw std::runtime_error(std::format("failed to write checksum to WAL file at {}", path));
    }
    // instead of writing 1 item of size bytes, we write size items of 1 byte
    // this allows to check for partial writes.
    if (fwrite(serialized.data(), 1, size, file) != size) {
        throw std::runtime_error(std::format("failed to write data to WAL file at {}", path));
    };
};

void WAL::flush() {
    std::lock_guard<std::mutex> lock(m);
    fflush(file);
    fsync(fd);
};

void WAL::recover(std::function<void(command::Command)> const& replay) {
    rewind(file);
    uint32_t size, crc;
    // read file, terminates at first failed / corrupted data.
    while (fread(&size, sizeof(size), 1, file) == 1) {
        if (fread(&crc, sizeof(crc), 1, file) != 1) {
            spdlog::warn("WAL corrupted, failed to parse crc");
            break;
        }
        std::string command(size, '\0');
        if (fread(command.data(), 1, size, file) != size) {
            spdlog::warn("WAL corrupted, command body partially written to");
            break;
        }
        if (const uint32_t expected = crc32(0, reinterpret_cast<const Bytef*>(command.data()), size); expected != crc) {
            spdlog::warn("WAL corrupted, mismatch in checksum");
            break;
        }
        replay(codec.deserialize(command));
    }
    // truncates where data is corrupted.
    const long pos = ftell(file);
    ftruncate(fd, pos);
};

void WAL::clear() {
    std::lock_guard<std::mutex> lock(m);
    spdlog::warn("clearing WAL file at: {}", path);
    fclose(file);
    std::remove(path.c_str());
};
} // namespace core