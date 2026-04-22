cmake_minimum_required(VERSION 3.31)
project(redis-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# set up for boost
find_package(Boost 1.81 REQUIRED COMPONENTS system)

# set up for zlib
find_package(ZLIB REQUIRED)

# set up for spdlog
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# set up for CLI 11
include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.4.2
)
FetchContent_MakeAvailable(cli11)

# set up for gtest
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# set up for redis-cpp
file(GLOB_RECURSE CPP_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(redis_cpp_lib
        ${CPP_SOURCES}
)

target_include_directories(redis_cpp_lib PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
)

target_compile_definitions(redis_cpp_lib PUBLIC BOOST_ASIO_HAS_CO_AWAIT)

target_link_libraries(redis_cpp_lib PUBLIC
        spdlog::spdlog
        Boost::system
        ZLIB::ZLIB
        CLI11::CLI11
)

add_executable(main
        src/main.cpp
)

target_link_libraries(main PRIVATE redis_cpp_lib)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(redis_cpp_tests
        ${TEST_SOURCES}
)

target_link_libraries(redis_cpp_tests
        PRIVATE
        redis_cpp_lib
        GTest::gtest_main
        ZLIB::ZLIB
)

include(GoogleTest)
gtest_discover_tests(redis_cpp_tests)