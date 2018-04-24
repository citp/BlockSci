cmake_minimum_required(VERSION 3.5)
project(blockscipy)

find_package(blocksci REQUIRED)

add_subdirectory(blocksci/pybind11)

file(GLOB PYTHON_INTERFACE_HEADERS "src/*.hpp")
file(GLOB PYTHON_INTERFACE_SOURCES "src/*.cpp")

pybind11_add_module(blocksci_interface ${PYTHON_INTERFACE_SOURCES} ${PYTHON_INTERFACE_HEADERS})

target_compile_options(blocksci_interface PRIVATE -Wall -Wextra -Wpedantic -Winstantiation-after-specialization)

target_link_libraries( blocksci_interface PUBLIC blocksci)

get_property(PYBIND11_PACKAGE_LOCATION GLOBAL PROPERTY pybind11_package_location)

source_group(blocksci_interface FILES ${PYTHON_INTERFACE_HEADERS} ${PYTHON_INTERFACE_SOURCES})
