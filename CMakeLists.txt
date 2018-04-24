cmake_minimum_required(VERSION 3.5)
project(blocksci)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(BlockSci_VERSION 0.5.0)

add_subdirectory(external)

add_subdirectory(src/blocksci)
add_subdirectory(src/parser)
add_subdirectory(src/mempool_recorder)
add_subdirectory(src/python-interface)
add_subdirectory(src/clusterer)
add_subdirectory(src/benchmark)
add_subdirectory(src/example)

include( CMakePackageConfigHelpers )
set(CONFIG_PACKAGE_INSTALL_DIR lib/cmake/blocksci)
