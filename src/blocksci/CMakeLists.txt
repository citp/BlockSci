file(GLOB BLOCKSCI_HEADERS "*.hpp")
file(GLOB BLOCKSCI_SOURCES "*.cpp")
file(GLOB SCRIPT_HEADERS "scripts/*.hpp")
file(GLOB SCRIPT_SOURCES "scripts/*.cpp")
file(GLOB BASE_HEADERS "base/*.hpp")
file(GLOB BASE_SOURCES "base/*.cpp")
file(GLOB CHAIN_HEADERS "chain/*.hpp")
file(GLOB CHAIN_SOURCES "chain/*.cpp")
file(GLOB CRYPTO_HEADERS "crypto/*.h")
file(GLOB CRYPTO_SOURCES "crypto/*.c")

set(ALL_SOURCES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES} ${SCRIPT_HEADERS} ${SCRIPT_SOURCES} ${BASE_HEADERS} ${BASE_SOURCES} ${CRYPTO_HEADERS} ${CRYPTO_SOURCES} ${CHAIN_HEADERS} ${CHAIN_SOURCES} )

add_library(blocksci SHARED ${ALL_SOURCES})

source_group(blocksci\\chain FILES ${CHAIN_HEADERS} ${CHAIN_SOURCES})
source_group(blocksci\\scripts FILES ${SCRIPT_HEADERS} ${SCRIPT_SOURCES})
source_group(blocksci\\crypto FILES ${CRYPTO_HEADERS} ${CRYPTO_SOURCES})
source_group(blocksci FILES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES})

target_link_libraries( blocksci boost )
target_link_libraries( blocksci sparsepp)
target_link_libraries( blocksci secp256k1)
target_link_libraries( blocksci openssl )
target_link_libraries( blocksci sqlite3)


target_include_directories(blocksci PUBLIC BEFORE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/blocksci>
)

install(TARGETS blocksci DESTINATION lib )
install(FILES ${BLOCKSCI_HEADERS} DESTINATION include/blocksci)
install(FILES ${SCRIPT_HEADERS} DESTINATION include/blocksci/scripts)
install(FILES ${BASE_HEADERS} DESTINATION include/blocksci/base)
install(FILES ${CHAIN_HEADERS} DESTINATION include/blocksci/chain)

