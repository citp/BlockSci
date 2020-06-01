cmake_minimum_required(VERSION 3.5)
project(mempool_recorder)

add_executable(mempool_recorder main.cpp file_writer.hpp)

target_compile_options(mempool_recorder PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(mempool_recorder PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-covered-switch-default -Wno-missing-prototypes -Wno-weak-vtables -Wno-unused-macros -Wno-padded -Wno-missing-variable-declarations)
endif()

target_link_libraries( mempool_recorder blocksci blocksci_internal)
target_link_libraries( mempool_recorder bitcoinapi_static)
target_link_libraries( mempool_recorder clipp)
target_link_libraries( mempool_recorder filesystem)
target_link_libraries( mempool_recorder json)

install(TARGETS mempool_recorder DESTINATION bin)
