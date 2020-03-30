cmake_minimum_required(VERSION 3.5)
project(integrity_check)

find_package(OpenSSL REQUIRED)

add_executable(blocksci_check_integrity main.cpp)

target_compile_options(blocksci_check_integrity PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(mempool_recorder PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-covered-switch-default -Wno-missing-prototypes -Wno-weak-vtables -Wno-unused-macros -Wno-padded)
endif()

target_link_libraries( blocksci_check_integrity OpenSSL::Crypto )
target_link_libraries( blocksci_check_integrity clipp)
target_link_libraries( blocksci_check_integrity blocksci blocksci_internal)
target_link_libraries( blocksci_check_integrity json)

install(TARGETS blocksci_check_integrity DESTINATION bin)
