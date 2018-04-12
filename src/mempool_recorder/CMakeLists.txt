add_executable(mempool_recorder main.cpp)

target_link_libraries( mempool_recorder blocksci_static)
target_link_libraries( mempool_recorder bitcoinapi_static)
target_link_libraries( mempool_recorder clipp)

install(TARGETS mempool_recorder DESTINATION bin)
