cmake_minimum_required(VERSION 3.13.4)

project(LLVM_STATIC_ANALYSIS_EXAMPLES
   LANGUAGES CXX)

find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(AddLLVM)

add_subdirectory(LiveVariables)
