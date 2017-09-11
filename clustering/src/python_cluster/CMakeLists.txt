
file(GLOB PYTHON_CLUSTER_HEADERS "*.hpp")
file(GLOB PYTHON_CLUSTER_SOURCES "*.cpp")

pybind11_add_module(cluster_python ${PYTHON_CLUSTER_SOURCES} ${PYTHON_CLUSTER_HEADERS})
target_link_libraries( cluster_python PUBLIC cluster)

install(TARGETS cluster_python DESTINATION ${CMAKE_SOURCE_DIR}/../Notebooks/blocksci)
