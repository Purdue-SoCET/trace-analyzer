cmake_minimum_required(VERSION 3.0)
project(trace_analyzer)

# For GTest
set(CMAKE_CXX_STANDARD 14)

# Include dirs and clangd support
include_directories(mpc)
include_directories(${PROJECT_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Libraries
# MPC - parsing combinator library
add_library(mpc OBJECT mpc/mpc.c)
# GTest - testing library
include(FetchContent)
# v1.13.0
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/b796f7d44681514f58a683a3a71ff17c94edb0c1.zip
)
FetchContent_MakeAvailable(googletest)

# Executables
add_executable(trace_analyzer src/main.c)
target_link_libraries(trace_analyzer mpc)

# Tests
enable_testing()

add_executable(parser_test src/parser_test.cpp)
target_link_libraries(parser_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(parser_test)
