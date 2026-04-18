#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#include "../include/command/command.hpp"
#include "../include/core/core.hpp"

using namespace core;
using response::CoreResp;

class DBCoreTest : public ::testing::Test {
  protected:
    wal_ptr wal = nullptr;
    std::unique_ptr<DBCore> core;

    void SetUp() override { core = std::make_unique<DBCore>(100, wal, 1000); }
};

TEST_F(DBCoreTest, SetThenGetReturnsValue) {
    command::Command set_cmd = command::SetCommand{"key1", "value1", 0};
    core->execute(set_cmd);

    command::Command get_cmd = command::GetCommand{"key1"};
    auto resp = core->execute(get_cmd);

    EXPECT_EQ(resp.type, CoreResp::RespType::VALUE);
    ASSERT_TRUE(resp.value.has_value());
    EXPECT_EQ(std::get<std::string>(resp.value.value()), "value1");
}

TEST_F(DBCoreTest, GetNonExistentKeyReturnsNil) {
    command::Command get_cmd = command::GetCommand{"missing"};
    auto resp = core->execute(get_cmd);

    EXPECT_EQ(resp.type, CoreResp::RespType::NIL);
}

TEST_F(DBCoreTest, DeleteExistingKeyReturnsOK) {
    command::Command set_cmd = command::SetCommand{"key1", "value1", 0};
    core->execute(set_cmd);

    command::Command del_cmd = command::DelCommand{"key1"};
    auto resp = core->execute(del_cmd);

    EXPECT_EQ(resp.type, CoreResp::RespType::OK);
}

TEST_F(DBCoreTest, DeleteMissingKeyReturnsError) {
    command::Command del_cmd = command::DelCommand{"missing"};
    auto resp = core->execute(del_cmd);

    EXPECT_EQ(resp.type, CoreResp::RespType::ERROR);
}

TEST_F(DBCoreTest, ExpiredKeyGetsEvicted) {
    command::Command set_cmd = command::SetCommand{"key1", "value1", 1}; // 1 ms TTL
    core->execute(set_cmd);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    core->evict();

    command::Command get_cmd = command::GetCommand{"key1"};
    auto resp = core->execute(get_cmd);

    EXPECT_EQ(resp.type, CoreResp::RespType::NIL);
}

TEST_F(DBCoreTest, SetOverridesExistingValue) {
    command::Command cmd1 = command::SetCommand{"key1", "v1", 0};
    command::Command cmd2 = command::SetCommand{"key1", "v2", 0};

    core->execute(cmd1);
    core->execute(cmd2);

    command::Command get = command::GetCommand{"key1"};
    auto resp = core->execute(get);

    ASSERT_TRUE(resp.value.has_value());
    EXPECT_EQ(std::get<std::string>(resp.value.value()), "v2");
}