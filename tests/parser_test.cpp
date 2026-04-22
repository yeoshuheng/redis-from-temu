//
// Created by Yeo Shu Heng on 14/4/26.
//

#include "../include/command/parser.hpp"

#include <gtest/gtest.h>

#include <cstring>
#include <variant>

using namespace command;

static void feed(Parser& p, const char* s) {
    p.feed(s, std::strlen(s));
}

TEST(ParserTest, BasicSet) {
    Parser parser;

    feed(parser, "*3\r\n"
                 "$3\r\nSET\r\n"
                 "$3\r\nkey\r\n"
                 "$5\r\nvalue\r\n");

    ASSERT_TRUE(parser.has_next_msg());
    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    const Command& cmd = *cmd_opt;

    ASSERT_TRUE(std::holds_alternative<SetCommand>(cmd));

    const auto& set = as<SetCommand>(cmd);
    EXPECT_EQ(set.key, "key");

    ASSERT_TRUE(std::holds_alternative<std::string>(set.value));
    EXPECT_EQ(std::get<std::string>(set.value), "value");
}

TEST(ParserTest, PingCommand) {
    Parser parser;

    feed(parser, "*1\r\n"
                 "$4\r\nPING\r\n");

    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    const Command& cmd = *cmd_opt;

    EXPECT_TRUE(std::holds_alternative<PingCommand>(cmd));
}

TEST(ParserTest, GetCommand) {
    Parser parser;

    feed(parser, "*2\r\n"
                 "$3\r\nGET\r\n"
                 "$3\r\nkey\r\n");

    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    const Command& cmd = *cmd_opt;

    ASSERT_TRUE(std::holds_alternative<GetCommand>(cmd));

    const auto& get = as<GetCommand>(cmd);
    EXPECT_EQ(get.key, "key");
}

TEST(ParserTest, DelCommand) {
    Parser parser;

    feed(parser, "*2\r\n"
                 "$3\r\nDEL\r\n"
                 "$3\r\nkey\r\n");

    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    const Command& cmd = *cmd_opt;

    ASSERT_TRUE(std::holds_alternative<DelCommand>(cmd));

    const auto& del = as<DelCommand>(cmd);
    EXPECT_EQ(del.key, "key");
}

TEST(ParserTest, PartialFeed) {
    Parser parser;

    feed(parser, "*3\r\n$3\r\nSE");

    EXPECT_FALSE(parser.has_next_msg());

    feed(parser, "T\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");

    ASSERT_TRUE(parser.has_next_msg());
    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    const auto& set = as<SetCommand>(*cmd_opt);
    EXPECT_EQ(set.key, "key");
    EXPECT_EQ(std::get<std::string>(set.value), "value");
}

TEST(ParserTest, MultipleCommands) {
    Parser parser;

    feed(parser, "*1\r\n$4\r\nPING\r\n"
                 "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n");

    ASSERT_TRUE(parser.has_next_msg());
    auto c1 = parser.next_msg();

    ASSERT_TRUE(parser.has_next_msg());
    auto c2 = parser.next_msg();

    ASSERT_TRUE(c1.has_value());
    ASSERT_TRUE(c2.has_value());

    EXPECT_TRUE(std::holds_alternative<PingCommand>(*c1));
    EXPECT_TRUE(std::holds_alternative<GetCommand>(*c2));

    const auto& get = as<GetCommand>(*c2);
    EXPECT_EQ(get.key, "key");
}

TEST(ParserTest, InvalidInput) {
    Parser parser;

    feed(parser, "NOT_A_RESP\r\n");

    auto cmd_opt = parser.next_msg();

    EXPECT_FALSE(cmd_opt.has_value());
}

TEST(ParserTest, IncompleteArrayHeader) {
    Parser parser;

    feed(parser, "*3\r\n$3\r\nSET\r\n");

    auto cmd_opt = parser.next_msg();

    EXPECT_FALSE(cmd_opt.has_value());
}

TEST(ParserTest, ChunkedFeedStress) {
    Parser parser;

    feed(parser, "*3\r\n$3\r\n");
    EXPECT_FALSE(parser.next_msg().has_value());

    feed(parser, "SET\r\n$3\r\nke");
    EXPECT_FALSE(parser.next_msg().has_value());

    feed(parser, "y\r\n$5\r\nvalue\r\n");

    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    ASSERT_TRUE(std::holds_alternative<SetCommand>(*cmd_opt));

    const auto& set = as<SetCommand>(*cmd_opt);

    EXPECT_EQ(set.key, "key");

    ASSERT_TRUE(std::holds_alternative<std::string>(set.value));
    EXPECT_EQ(std::get<std::string>(set.value), "value");
}

TEST(ParserTest, DrainMultipleMessages) {
    Parser parser;

    feed(parser, "*1\r\n$4\r\nPING\r\n"
                 "*1\r\n$4\r\nPING\r\n"
                 "*1\r\n$4\r\nPING\r\n");

    for (int i = 0; i < 3; i++) {
        ASSERT_TRUE(parser.has_next_msg());
        auto cmd = parser.next_msg();
        ASSERT_TRUE(cmd.has_value());
        EXPECT_TRUE(std::holds_alternative<PingCommand>(*cmd));
    }

    EXPECT_FALSE(parser.has_next_msg());
}