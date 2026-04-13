cmake_minimum_required(VERSION 3.31)
project(redis-cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    command/parser.cpp
)

set(HEADERS
    command/parser.hpp
    command/command.hpp
    commons/utils.hpp
)

add_executable(
        main.cpp
        command/parser.cpp
        ${HEADERS}
)