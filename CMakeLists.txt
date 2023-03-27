cmake_minimum_required(VERSION 3.0)
project(trace_analyzer)

# Necessary for parallelization
set(CMAKE_CXX_STANDARD 17)

# Include dirs and clangd support
include_directories(${PROJECT_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Used for valgrind support on asicfab
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -gdwarf-4")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-4")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-exceptions -fno-rtti")

# Sources
set(SRCS src/parser.cpp src/analyzer.cpp src/ObjBuffer.cpp)
set(MAIN_SRCS src/main.cpp ${SRCS})
set(TEST_SRCS src/parser_test.cpp src/analyzer_test.cpp ${SRCS})

# Libraries
# MPC - parsing combinator library
include_directories(mpc)
add_library(mpc OBJECT mpc/mpc.c)
# LLVM - used to disassemble instructions
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(llvm_libs Support Target RISCVAsmParser RISCVCodeGen RISCVDesc RISCVDisassembler RISCVInfo MC Object)
# TBB - threading will need to be compiled from source on asicfab (https://github.com/oneapi-src/oneTBB)
find_package(TBB)

set(SRC_LIBRARIES mpc TBB::tbb ${llvm_libs})

# Executable
add_executable(trace_analyzer ${MAIN_SRCS})
target_link_libraries(trace_analyzer ${SRC_LIBRARIES})

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

    add_compile_definitions(CMAKE_TXT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    enable_testing()

    add_executable(parser_test ${TEST_SRCS})
    target_link_libraries(parser_test ${SRC_LIBRARIES} GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(parser_test)
endif()
