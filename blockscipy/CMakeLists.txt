cmake_minimum_required(VERSION 3.8.2)
project(blockscipy)

set (CMAKE_CXX_STANDARD 17)

find_package(blocksci REQUIRED)

add_subdirectory(blocksci/pybind11)

file(GLOB_RECURSE PYTHON_CLUSTER_HEADERS "src/cluster/*.hpp")
file(GLOB_RECURSE PYTHON_CLUSTER_SOURCES "src/cluster/*.cpp")

file(GLOB_RECURSE PYTHON_CHAIN_HEADERS "src/chain/*.hpp")
file(GLOB_RECURSE PYTHON_CHAIN_SOURCES "src/chain/*.cpp")

file(GLOB_RECURSE PYTHON_SCRIPTS_HEADERS "src/scripts/*.hpp")
file(GLOB_RECURSE PYTHON_SCRIPTS_SOURCES "src/scripts/*.cpp")

file(GLOB PYTHON_INTERFACE_HEADERS "src/*.hpp")
file(GLOB PYTHON_INTERFACE_SOURCES "src/*.cpp")

pybind11_add_module(_blocksci ${PYTHON_CLUSTER_HEADERS} ${PYTHON_CLUSTER_SOURCES} ${PYTHON_CHAIN_HEADERS} ${PYTHON_SCRIPTS_HEADERS} ${PYTHON_SCRIPTS_SOURCES} ${PYTHON_CHAIN_SOURCES} ${PYTHON_INTERFACE_SOURCES} ${PYTHON_INTERFACE_HEADERS})

target_compile_options(_blocksci PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries( _blocksci PUBLIC blocksci)

target_include_directories (_blocksci PRIVATE external)
target_include_directories (_blocksci PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

get_property(PYBIND11_PACKAGE_LOCATION GLOBAL PROPERTY pybind11_package_location)
