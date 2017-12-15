file(GLOB PARSER_HEADERS "*.hpp")
file(GLOB PARSER_SOURCES "*.cpp")

add_executable(blocksci_parser ${PARSER_SOURCES} ${PARSER_HEADERS})

source_group(blocksci_parser FILES ${PARSER_SOURCES} ${PARSER_HEADERS})

target_link_libraries( blocksci_parser bitcoinapi)
target_link_libraries( blocksci_parser hyperleveldb)
target_link_libraries( blocksci_parser clipp)
target_link_libraries( blocksci_parser cereal )
target_link_libraries( blocksci_parser blocksci)

install(TARGETS blocksci_parser DESTINATION bin)
