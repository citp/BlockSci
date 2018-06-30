file(GLOB EXAMPLE_HEADERS "*.hpp")
file(GLOB EXAMPLE_SOURCES "*.cpp")

add_executable(example EXCLUDE_FROM_ALL ${EXAMPLE_SOURCES} ${EXAMPLE_HEADERS})

target_link_libraries( example cluster)
