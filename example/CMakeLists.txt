cmake_minimum_required(VERSION 3.5)
project(blocksci_example)

file(GLOB EXAMPLE_HEADERS "*.hpp")
file(GLOB EXAMPLE_SOURCES "*.cpp")

add_executable(blocksci_example EXCLUDE_FROM_ALL ${EXAMPLE_SOURCES} ${EXAMPLE_HEADERS})

target_link_libraries( blocksci_example blocksci)
