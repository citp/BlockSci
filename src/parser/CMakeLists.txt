find_package(OpenSSL REQUIRED)
find_package(Sparsehash REQUIRED)

file(GLOB PARSER_HEADERS "*.hpp")
file(GLOB PARSER_SOURCES "*.cpp")

set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS "-Wno-reserved-id-macro -Wno-sign-conversion -Wno-extra-semi -Wno-undefined-func-template")

add_executable(blocksci_parser ${PARSER_SOURCES} ${PARSER_HEADERS})

source_group(blocksci_parser FILES ${PARSER_SOURCES} ${PARSER_HEADERS})

target_link_libraries( blocksci_parser OpenSSL::Crypto )
target_link_libraries( blocksci_parser bitcoinapi_static)
target_link_libraries( blocksci_parser clipp)
target_link_libraries( blocksci_parser blocksci_static)
target_link_libraries( blocksci_parser secp256k1)
target_link_libraries( blocksci_parser sparsehash)

install(TARGETS blocksci_parser DESTINATION bin)
