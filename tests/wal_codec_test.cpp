//
// Created by Yeo Shu Heng on 16/4/26.
//
#include <gtest/gtest.h>

#include "include/command/command.hpp"
#include "include/wal/wal_codec.hpp"

using namespace wal;

TEST(WALCodecTest, SerializeDeserialize_Int64) {
    stored_value val = int64_t(123456789);

    std::string serialized = WALCodec::serialize_stored_value(val);
    const char* ptr = serialized.data();

    stored_value out = WALCodec::deserialize_stored_value(ptr);

    ASSERT_TRUE(std::holds_alternative<int64_t>(out));
    EXPECT_EQ(std::get<int64_t>(out), 123456789);
}

TEST(WALCodecTest, SerializeDeserialize_Double) {
    stored_value val = double(3.1415926535);

    std::string serialized = WALCodec::serialize_stored_value(val);
    const char* ptr = serialized.data();

    stored_value out = WALCodec::deserialize_stored_value(ptr);

    ASSERT_TRUE(std::holds_alternative<double>(out));
    EXPECT_DOUBLE_EQ(std::get<double>(out), 3.1415926535);
}

TEST(WALCodecTest, SerializeDeserialize_Float) {
    stored_value val = float(1.2345f);

    std::string serialized = WALCodec::serialize_stored_value(val);
    const char* ptr = serialized.data();

    stored_value out = WALCodec::deserialize_stored_value(ptr);

    ASSERT_TRUE(std::holds_alternative<float>(out));
    EXPECT_FLOAT_EQ(std::get<float>(out), 1.2345f);
}

TEST(WALCodecTest, SerializeDeserialize_String) {
    stored_value val = std::string("hello world");

    std::string serialized = WALCodec::serialize_stored_value(val);
    const char* ptr = serialized.data();

    stored_value out = WALCodec::deserialize_stored_value(ptr);

    ASSERT_TRUE(std::holds_alternative<std::string>(out));
    EXPECT_EQ(std::get<std::string>(out), "hello world");
}

TEST(WALCodecTest, SerializeDeserialize_EmptyString) {
    stored_value val = std::string("");

    std::string serialized = WALCodec::serialize_stored_value(val);
    const char* ptr = serialized.data();

    stored_value out = WALCodec::deserialize_stored_value(ptr);

    ASSERT_TRUE(std::holds_alternative<std::string>(out));
    EXPECT_EQ(std::get<std::string>(out), "");
}

TEST(WALCodecTest, SerializeDeserialize_PingCommand) {
    command::Command cmd = command::PingCommand{};

    std::string serialized = WALCodec::serialize(cmd);
    command::Command out = WALCodec::deserialize(serialized);

    ASSERT_TRUE(std::holds_alternative<command::PingCommand>(out));
}

TEST(WALCodecTest, SerializeDeserialize_SetCommand_Int) {
    command::Command cmd = command::SetCommand{"key1", int64_t(42), 100};

    std::string serialized = WALCodec::serialize(cmd);
    command::Command out = WALCodec::deserialize(serialized);

    ASSERT_TRUE(std::holds_alternative<command::SetCommand>(out));

    auto& result = std::get<command::SetCommand>(out);
    EXPECT_EQ(result.key, "key1");
    EXPECT_EQ(std::get<int64_t>(result.value), 42);
    EXPECT_EQ(result.ttl_ms, 100);
}

TEST(WALCodecTest, SerializeDeserialize_SetCommand_String) {
    command::Command cmd = command::SetCommand{"k", std::string("value"), 0};

    std::string serialized = WALCodec::serialize(cmd);
    command::Command out = WALCodec::deserialize(serialized);

    ASSERT_TRUE(std::holds_alternative<command::SetCommand>(out));

    auto& result = std::get<command::SetCommand>(out);
    EXPECT_EQ(result.key, "k");
    EXPECT_EQ(std::get<std::string>(result.value), "value");
    EXPECT_EQ(result.ttl_ms, 0);
}

TEST(WALCodecTest, SerializeDeserialize_SetCommand_Double) {
    command::Command cmd = command::SetCommand{"pi", 3.14, 0};

    std::string serialized = WALCodec::serialize(cmd);
    command::Command out = WALCodec::deserialize(serialized);

    ASSERT_TRUE(std::holds_alternative<command::SetCommand>(out));

    auto& result = std::get<command::SetCommand>(out);
    EXPECT_EQ(result.key, "pi");
    EXPECT_DOUBLE_EQ(std::get<double>(result.value), 3.14);
}

TEST(WALCodecTest, SerializeDeserialize_DelCommand) {
    command::Command cmd = command::DelCommand{"delete_me"};

    std::string serialized = WALCodec::serialize(cmd);
    command::Command out = WALCodec::deserialize(serialized);

    ASSERT_TRUE(std::holds_alternative<command::DelCommand>(out));

    auto& result = std::get<command::DelCommand>(out);
    EXPECT_EQ(result.key, "delete_me");
}

TEST(WALCodecTest, SerializeDeserialize_GetCommand) {
    command::Command cmd = command::GetCommand{"get_me"};

    std::string serialized = WALCodec::serialize(cmd);
    command::Command out = WALCodec::deserialize(serialized);

    ASSERT_TRUE(std::holds_alternative<command::GetCommand>(out));

    auto& result = std::get<command::GetCommand>(out);
    EXPECT_EQ(result.key, "get_me");
}

TEST(WALCodecTest, LargeStringValue) {
    std::string large(100000, 'x');
    command::Command cmd = command::SetCommand{"big", large, 0};

    std::string serialized = WALCodec::serialize(cmd);
    command::Command out = WALCodec::deserialize(serialized);

    auto& result = std::get<command::SetCommand>(out);
    EXPECT_EQ(std::get<std::string>(result.value), large);
}

TEST(WALCodecTest, PointerAdvancesCorrectly) {
    stored_value val1 = int64_t(1);
    stored_value val2 = double(2.0);

    std::string s1 = WALCodec::serialize_stored_value(val1);
    std::string s2 = WALCodec::serialize_stored_value(val2);

    std::string combined = s1 + s2;

    const char* ptr = combined.data();

    auto out1 = WALCodec::deserialize_stored_value(ptr);
    auto out2 = WALCodec::deserialize_stored_value(ptr);

    EXPECT_EQ(std::get<int64_t>(out1), 1);
    EXPECT_DOUBLE_EQ(std::get<double>(out2), 2.0);
}