//
// Created by Yeo Shu Heng on 16/4/26.
//
#include "../../include/wal/wal.hpp"

#include "spdlog/spdlog.h"
#include <zlib.h>

namespace wal {
WAL::WAL(const std::string& path) : path(path) {
    // https://en.cppreference.com/w/cpp/io/c/fopen.html
    // try to open with read + binary first
    file = fopen(path.c_str(), "r+b");
    if (!file) {
        spdlog::warn(std::format("failed to find WAL file at {}, starting new file", path));
        file = fopen(path.c_str(), "w+b");
    }
    setvbuf(file, nullptr, _IOFBF, RAM_BUFFER_BYTES);
    fd = fileno(file);
};

WAL::~WAL() {
    if (file)
        fclose(file);
}

void WAL::append(const command::Command& cmd) {
    const std::string serialized = codec.serialize(cmd);
    const uint32_t size = serialized.size();
    // create checksum
    const uint32_t crc = crc32(0, reinterpret_cast<const Bytef*>(serialized.data()), size);
    std::lock_guard<std::mutex> lock(m);
    fseek(file, 0, SEEK_END);
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
    std::lock_guard<std::mutex> lock(m);
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
        if (const uint32_t expected =
                crc32(0, reinterpret_cast<const Bytef*>(command.data()), size);
            expected != crc) {
            spdlog::warn("WAL corrupted, mismatch in checksum");
            break;
        }
        replay(codec.deserialize(command));
    }
    // truncates where data is corrupted.
    fflush(file);
    const long pos = ftell(file);
    ftruncate(fd, pos);
    fseek(file, 0, SEEK_END);
};

void WAL::clear() {
    std::lock_guard<std::mutex> lock(m);
    spdlog::warn("clearing WAL file at: {}", path);
    fclose(file);
    file = nullptr;
    fd = -1;
    std::remove(path.c_str());
};
} // namespace wal