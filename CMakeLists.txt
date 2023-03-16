cmake_minimum_required(VERSION 3.0)
project(trace_analyzer)

# For GTest
set(CMAKE_CXX_STANDARD 14)

# Include dirs and clangd support
include_directories(mpc)
include_directories(${PROJECT_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Used for valgrind support on asicfab
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-4")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-4")

# Libraries
# MPC - parsing combinator library
add_library(mpc OBJECT mpc/mpc.c)
# Executable
set(MAIN_SRCS src/main.c
    src/parser.c)
add_executable(trace_analyzer ${MAIN_SRCS})
target_link_libraries(trace_analyzer mpc)

# Tests
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # GTest - testing library
    include(FetchContent)
    # v1.13.0
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/b796f7d44681514f58a683a3a71ff17c94edb0c1.zip
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    set(TEST_SRCS src/parser_test.cpp
        src/parser.c)
    add_executable(parser_test ${TEST_SRCS})
    target_link_libraries(parser_test GTest::gtest_main mpc)

    include(GoogleTest)
    gtest_discover_tests(parser_test)
endif()
