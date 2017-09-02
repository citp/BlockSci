file(GLOB PYTHON_INTERFACE_HEADERS "*.hpp")
file(GLOB PYTHON_INTERFACE_SOURCES "*.cpp")


add_compile_options(-Wno-format-pedantic)
add_compile_options(-Wno-zero-length-array)
add_compile_options(-Wno-nested-anon-types)

pybind11_add_module(blocksci_interface ${PYTHON_INTERFACE_SOURCES} ${PYTHON_INTERFACE_HEADERS})
target_link_libraries( blocksci_interface PUBLIC blocksci)

install(TARGETS blocksci_interface DESTINATION ${CMAKE_SOURCE_DIR}/Notebooks/blocksci)
