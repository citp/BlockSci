cmake_minimum_required(VERSION 3.5)
project(blocksci_clusterer)

add_executable(blocksci_clusterer main.cpp)

target_compile_options(blocksci_clusterer PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(blocksci_clusterer PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-covered-switch-default -Wno-missing-prototypes -Wno-weak-vtables -Wno-unused-macros -Wno-padded -Wno-missing-variable-declarations)
endif()

target_link_libraries( blocksci_clusterer dset)
target_link_libraries( blocksci_clusterer clipp)
target_link_libraries( blocksci_clusterer blocksci)

install(TARGETS blocksci_clusterer DESTINATION bin)
