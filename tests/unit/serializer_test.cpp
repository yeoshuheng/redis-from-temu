//
// Created by Yeo Shu Heng on 18/4/26.
//
#include <gtest/gtest.h>

#include "../../include/resp/serializer.hpp"

using namespace response;
using response::stored_value;

class SerializerTest : public ::testing::Test {
  protected:
    Serializer serializer;
};

TEST_F(SerializerTest, SerializeOK) {
    CoreResp resp{CoreResp::RespType::OK, std::nullopt, "OK"};

    EXPECT_EQ(serializer.serialize(resp), "+OK\r\n");
}

TEST_F(SerializerTest, SerializeError) {
    CoreResp resp{CoreResp::RespType::ERROR, std::nullopt, "something went wrong"};

    EXPECT_EQ(serializer.serialize(resp), "-something went wrong\r\n");
}

TEST_F(SerializerTest, SerializeNil) {
    CoreResp resp{CoreResp::RespType::NIL, std::nullopt, ""};

    EXPECT_EQ(serializer.serialize(resp), "$-1\r\n");
}

TEST_F(SerializerTest, SerializeStringValue) {
    CoreResp resp{CoreResp::RespType::VALUE, stored_value(std::string("hello")), ""};

    EXPECT_EQ(serializer.serialize(resp), "$5\r\nhello\r\n");
}

TEST_F(SerializerTest, SerializeEmptyString) {
    CoreResp resp{CoreResp::RespType::VALUE, stored_value(std::string("")), ""};

    EXPECT_EQ(serializer.serialize(resp), "$0\r\n\r\n");
}

TEST_F(SerializerTest, SerializeDoubleValue) {
    CoreResp resp{CoreResp::RespType::VALUE, stored_value(3.14), ""};

    // depends on std::to_string formatting
    EXPECT_EQ(serializer.serialize(resp), "$8\r\n3.140000\r\n");
}

TEST_F(SerializerTest, SerializeValueWithoutPayload) {
    CoreResp resp{CoreResp::RespType::VALUE, std::nullopt, ""};

    EXPECT_EQ(serializer.serialize(resp), "$-1\r\n");
}

TEST_F(SerializerTest, ValueLengthMatchesPayload) {
    CoreResp resp{CoreResp::RespType::VALUE, stored_value(std::string("abc")), ""};

    std::string out = serializer.serialize(resp);

    EXPECT_TRUE(out.starts_with("$3\r\n"));
    EXPECT_TRUE(out.ends_with("\r\n"));
}
