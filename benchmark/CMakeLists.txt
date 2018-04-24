cmake_minimum_required(VERSION 3.5)
project(blocksci_benchmark)

find_package(blocksci REQUIRED)

add_executable(blocksci_benchmark EXCLUDE_FROM_ALL main.cpp)

target_compile_options(blocksci_benchmark PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(blocksci_benchmark PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-covered-switch-default -Wno-missing-prototypes -Wno-weak-vtables -Wno-unused-macros -Wno-padded)
endif()

target_link_libraries(blocksci_benchmark blocksci)
target_link_libraries(blocksci_benchmark clipp)
