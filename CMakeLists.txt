cmake_minimum_required(VERSION 3.4.3)

find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(AddLLVM)

add_subdirectory(LiveVariables)
