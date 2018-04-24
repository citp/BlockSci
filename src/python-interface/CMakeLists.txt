file(GLOB PYTHON_INTERFACE_HEADERS "*.hpp")
file(GLOB PYTHON_INTERFACE_SOURCES "*.cpp")

pybind11_add_module(blocksci_interface ${PYTHON_INTERFACE_SOURCES} ${PYTHON_INTERFACE_HEADERS})

target_compile_options(blocksci_interface PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries( blocksci_interface PUBLIC blocksci)

get_property(PYBIND11_PACKAGE_LOCATION GLOBAL PROPERTY pybind11_package_location)

install(TARGETS blocksci_interface DESTINATION ${CMAKE_SOURCE_DIR}/Notebooks/blocksci)
install(DIRECTORY ${PYBIND11_PACKAGE_LOCATION} DESTINATION ${CMAKE_SOURCE_DIR}/Notebooks/blocksci)

source_group(blocksci_interface FILES ${PYTHON_INTERFACE_HEADERS} ${PYTHON_INTERFACE_SOURCES})
