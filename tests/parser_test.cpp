//
// Created by Yeo Shu Heng on 14/4/26.
//

#include "../command/parser.hpp"

#include <gtest/gtest.h>

#include <cstring>

using namespace command;

static void feed(Parser& p, const char* s) {
    p.feed(s, std::strlen(s));
}

TEST(ParserTest, BasicSet) {
    Parser parser;

    feed(parser,
         "*3\r\n"
         "$3\r\nSET\r\n"
         "$3\r\nkey\r\n"
         "$5\r\nvalue\r\n");

    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    const Command& cmd = *cmd_opt;

    EXPECT_EQ(cmd.type, CommandType::SET);
    ASSERT_EQ(cmd.args.size(), 2);
    EXPECT_EQ(cmd.args[0], "key");
    EXPECT_EQ(cmd.args[1], "value");
}

TEST(ParserTest, PingCommand) {
    Parser parser;

    feed(parser,
         "*1\r\n"
         "$4\r\nPING\r\n");

    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    const Command& cmd = *cmd_opt;

    EXPECT_EQ(cmd.type, CommandType::PING);
    EXPECT_TRUE(cmd.args.empty());
}

TEST(ParserTest, PartialFeed) {
    Parser parser;

    feed(parser, "*3\r\n$3\r\nSE");

    // incomplete
    EXPECT_FALSE(parser.next_msg().has_value());

    feed(parser, "T\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");

    auto cmd_opt = parser.next_msg();
    ASSERT_TRUE(cmd_opt.has_value());

    const Command& cmd = *cmd_opt;

    EXPECT_EQ(cmd.type, CommandType::SET);
    EXPECT_EQ(cmd.args[0], "key");
}

TEST(ParserTest, MultipleCommands) {
    Parser parser;

    feed(parser,
         "*1\r\n$4\r\nPING\r\n"
         "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n");

    auto c1 = parser.next_msg();
    auto c2 = parser.next_msg();

    ASSERT_TRUE(c1.has_value());
    ASSERT_TRUE(c2.has_value());

    EXPECT_EQ(c1->type, CommandType::PING);
    EXPECT_EQ(c2->type, CommandType::GET);

    ASSERT_FALSE(c2->args.empty());
    EXPECT_EQ(c2->args[0], "key");
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

    EXPECT_EQ(cmd_opt->type, CommandType::SET);
    EXPECT_EQ(cmd_opt->args[0], "key");
}

TEST(ParserTest, DrainMultipleMessages) {
    Parser parser;

    feed(parser,
         "*1\r\n$4\r\nPING\r\n"
         "*1\r\n$4\r\nPING\r\n"
         "*1\r\n$4\r\nPING\r\n");

    for (int i = 0; i < 3; i++) {
        auto cmd = parser.next_msg();
        ASSERT_TRUE(cmd.has_value());
        EXPECT_EQ(cmd->type, CommandType::PING);
    }

    EXPECT_FALSE(parser.next_msg().has_value());
}