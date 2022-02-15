cmake_minimum_required(VERSION 3.1)
project(ScalarReplAggregates-kunli3)

set(CMAKE_CXX_STANDARD 11)

SET(ENV{LLVM_DIR} /Users/likun/Projects/VSCodeProjects/Course/CS526/llvm-8.0.1/lib/cmake)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

add_subdirectory(pass)
