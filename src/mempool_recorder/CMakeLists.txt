file(GLOB MEMPOOL_RECORDER_HEADERS "*.hpp")
file(GLOB MEMPOOL_RECORDER_SOURCES "*.cpp")

FIND_PACKAGE(CURL REQUIRED)

add_executable(mempool_recorder ${MEMPOOL_RECORDER_SOURCES} ${MEMPOOL_RECORDER_HEADERS})

target_link_libraries( mempool_recorder blocksci)
target_link_libraries( mempool_recorder bitcoinapi)
target_link_libraries( mempool_recorder curl)
target_link_libraries( mempool_recorder ${Boost_LIBRARIES})

install(TARGETS mempool_recorder DESTINATION bin)
