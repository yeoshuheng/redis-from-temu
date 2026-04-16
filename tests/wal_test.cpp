//
// Created by Yeo Shu Heng on 16/4/26.
//
#include <gtest/gtest.h>

#include "include/core/wal.hpp"

#include <filesystem>

using namespace core;

TEST(WALTest, DetectsChecksumMismatch) {
    const std::string path = "test_wal_crc.log";
    std::filesystem::remove(path);

    core::WAL wal(path);

    command::SetCommand cmd{"key", int64_t(999), 0};
    wal.append(cmd);
    wal.flush();

    // corrupt file
    {
        FILE* f = fopen(path.c_str(), "r+b");
        fseek(f, -1, SEEK_END);
        char byte;
        fread(&byte, 1, 1, f);
        byte ^= 0xFF; // flip bits
        fseek(f, -1, SEEK_END);
        fwrite(&byte, 1, 1, f);
        fclose(f);
    }

    std::vector<command::Command> recovered;

    wal.recover([&](command::Command cmd) {
        recovered.push_back(std::move(cmd));
    });

    // Corrupted entry should not be replayed
    ASSERT_TRUE(recovered.empty() || recovered.size() == 0);
}

TEST(WALTest, ClearDeletesFile) {
    const std::string path = "test_wal_clear.log";
    std::filesystem::remove(path);

    {
        core::WAL wal(path);
        wal.append(command::PingCommand{});
        wal.flush();
        wal.clear();
    }

    ASSERT_FALSE(std::filesystem::exists(path));
}

