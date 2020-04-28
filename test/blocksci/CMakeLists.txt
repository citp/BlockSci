cmake_minimum_required(VERSION 3.5)
project(blocksci_unittest)

file(GLOB SRCS *.cpp)

add_executable(blocksci_unittest EXCLUDE_FROM_ALL ${SRCS})

# target_compile_features(blocksci_unittest PUBLIC cxx_std_11)
target_compile_options(blocksci_unittest PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(blocksci_unittest PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-covered-switch-default -Wno-missing-prototypes -Wno-weak-vtables -Wno-unused-macros -Wno-padded -Wno-global-constructors)
endif()

target_link_libraries(blocksci_unittest blocksci)
target_link_libraries(blocksci_unittest clipp)
target_link_libraries(blocksci_unittest gtest)
