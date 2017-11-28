file(GLOB PARSER_HEADERS "*.hpp")
file(GLOB PARSER_SOURCES "*.cpp")

add_executable(blocksci_parser ${PARSER_SOURCES} ${PARSER_HEADERS})

source_group(blocksci_parser FILES ${PARSER_SOURCES} ${PARSER_HEADERS})

target_link_libraries( blocksci_parser profiler)
target_link_libraries( blocksci_parser bitcoinapi sparsepp)
target_link_libraries( blocksci_parser secp256k1)
target_link_libraries( blocksci_parser leveldb)
target_link_libraries( blocksci_parser ${Boost_LIBRARIES})
target_link_libraries( blocksci_parser blocksci)

install(TARGETS blocksci_parser DESTINATION bin)
