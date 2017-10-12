file(GLOB BLOCKSCI_HEADERS "*.hpp")
file(GLOB BLOCKSCI_SOURCES "*.cpp")
file(GLOB ADDRESS_HEADERS "address/*.hpp")
file(GLOB ADDRESS_SOURCES "address/*.cpp")
file(GLOB SCRIPT_HEADERS "scripts/*.hpp")
file(GLOB SCRIPT_SOURCES "scripts/*.cpp")
file(GLOB CHAIN_HEADERS "chain/*.hpp")
file(GLOB CHAIN_SOURCES "chain/*.cpp")

set(ALL_SOURCES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES} ${ADDRESS_HEADERS} ${ADDRESS_SOURCES}  ${SCRIPT_HEADERS} ${SCRIPT_SOURCES} ${CHAIN_HEADERS} ${CHAIN_SOURCES} )

add_library(blocksci SHARED ${ALL_SOURCES})

source_group(blocksci\\chain FILES ${CHAIN_HEADERS} ${CHAIN_SOURCES})
source_group(blocksci\\address FILES ${ADDRESS_HEADERS} ${ADDRESS_SOURCES})
source_group(blocksci\\script FILES ${SCRIPT_HEADERS} ${SCRIPT_SOURCES})
source_group(blocksci FILES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES})

target_link_libraries( blocksci boost )
target_link_libraries( blocksci sparsepp)
target_link_libraries( blocksci secp256k1)
target_link_libraries( blocksci openssl )
target_link_libraries( blocksci SQLiteCppInclude)


target_include_directories(blocksci PUBLIC BEFORE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/blocksci>
)

install(TARGETS blocksci DESTINATION lib )
install(FILES ${BLOCKSCI_HEADERS} DESTINATION include/blocksci)
install(FILES ${ADDRESS_HEADERS} DESTINATION include/blocksci/address)
install(FILES ${SCRIPT_HEADERS} DESTINATION include/blocksci/scripts)
install(FILES ${CHAIN_HEADERS} DESTINATION include/blocksci/chain)

