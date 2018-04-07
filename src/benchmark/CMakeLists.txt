cmake_minimum_required(VERSION 3.9)
project(blocksci_benchmark)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -march=native -mtune=native")

add_executable(blocksci_benchmark main.cpp)
target_link_libraries(blocksci_benchmark blocksci_static)
target_link_libraries(blocksci_benchmark clipp)

install(TARGETS blocksci_benchmark DESTINATION bin)
