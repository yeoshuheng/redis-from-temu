//
// Created by Yeo Shu Heng on 16/4/26.
//
#include <gtest/gtest.h>

#include "include/command/command.hpp"
#include "include/wal/wal_codec.hpp"

using namespace wal;

class WALCodecTest : public ::testing::Test {
  protected:
    static stored_value roundtrip(const stored_value& val) {
        const std::string serialized = WALCodec::serialize_stored_value(val);

        const char* begin = serialized.data();
        const char* end = begin + serialized.size();
        const char* ptr = begin;

        auto out = WALCodec::deserialize_stored_value(ptr, end);

        // Ensure full consumption (IMPORTANT)
        EXPECT_EQ(ptr, end);

        return out;
    }
};
TEST_F(WALCodecTest, Int64) {
    auto out = roundtrip(int64_t(123456789));
    ASSERT_TRUE(std::holds_alternative<int64_t>(out));
    EXPECT_EQ(std::get<int64_t>(out), 123456789);
}

TEST_F(WALCodecTest, Double) {
    auto out = roundtrip(double(3.1415926535));
    ASSERT_TRUE(std::holds_alternative<double>(out));
    EXPECT_DOUBLE_EQ(std::get<double>(out), 3.1415926535);
}

TEST_F(WALCodecTest, Float) {
    auto out = roundtrip(float(1.2345f));
    ASSERT_TRUE(std::holds_alternative<float>(out));
    EXPECT_FLOAT_EQ(std::get<float>(out), 1.2345f);
}

TEST_F(WALCodecTest, String) {
    auto out = roundtrip(std::string("hello world"));
    ASSERT_TRUE(std::holds_alternative<std::string>(out));
    EXPECT_EQ(std::get<std::string>(out), "hello world");
}

TEST_F(WALCodecTest, EmptyString) {
    auto out = roundtrip(std::string(""));
    ASSERT_TRUE(std::holds_alternative<std::string>(out));
    EXPECT_EQ(std::get<std::string>(out), "");
}