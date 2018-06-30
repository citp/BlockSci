file(GLOB PYTHON_INTERFACE_HEADERS "*.hpp")
file(GLOB PYTHON_INTERFACE_SOURCES "*.cpp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nested-anon-types")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-id-macro -Wno-deprecated -Wno-range-loop-analysis -Wno-covered-switch-default -Wno-undef -Wno-undefined-reinterpret-cast -Wno-float-equal")

pybind11_add_module(blocksci_interface ${PYTHON_INTERFACE_SOURCES} ${PYTHON_INTERFACE_HEADERS})
target_link_libraries( blocksci_interface PUBLIC blocksci)

install(TARGETS blocksci_interface DESTINATION ${CMAKE_SOURCE_DIR}/Notebooks/blocksci)
