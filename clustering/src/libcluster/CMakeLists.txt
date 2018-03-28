file(GLOB CLUSTER_HEADERS "*.hpp")
file(GLOB CLUSTER_SOURCES "*.cpp")

add_compile_options(-Wno-zero-length-array)
add_compile_options(-Wno-format-pedantic)

add_library(cluster SHARED ${CLUSTER_SOURCES} ${CLUSTER_HEADERS})

target_link_libraries( cluster blocksci)

target_include_directories(cluster PUBLIC ${CMAKE_SOURCE_DIR}/src)

install(TARGETS cluster DESTINATION lib)
