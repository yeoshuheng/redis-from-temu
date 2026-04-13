//
// Created by Yeo Shu Heng on 14/4/26.
//

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%H:%M:%S] [%l] %v");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}