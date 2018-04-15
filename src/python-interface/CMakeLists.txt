file(GLOB PYTHON_INTERFACE_HEADERS "*.hpp")
file(GLOB PYTHON_INTERFACE_SOURCES "*.cpp")

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
add_compile_options(-Wno-nested-anon-types)
endif()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-id-macro -Wno-deprecated -Wno-range-loop-analysis -Wno-covered-switch-default -Wno-undef -Wno-undefined-reinterpret-cast -Wno-float-equal")

pybind11_add_module(blocksci_interface ${PYTHON_INTERFACE_SOURCES} ${PYTHON_INTERFACE_HEADERS})
target_link_libraries( blocksci_interface PUBLIC blocksci_static)

install(TARGETS blocksci_interface DESTINATION ${CMAKE_SOURCE_DIR}/Notebooks/blocksci)

source_group(blocksci_interface FILES ${PYTHON_INTERFACE_HEADERS} ${PYTHON_INTERFACE_SOURCES})
