set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")

file(GLOB BLOCKSCI_HEADERS "*.hpp")
file(GLOB BLOCKSCI_SOURCES "*.cpp")
file(GLOB ADDRESS_HEADERS "address/*.hpp")
file(GLOB ADDRESS_SOURCES "address/*.cpp")
file(GLOB SCRIPT_HEADERS "scripts/*.hpp")
file(GLOB SCRIPT_SOURCES "scripts/*.cpp")
file(GLOB CHAIN_HEADERS "chain/*.hpp")
file(GLOB CHAIN_SOURCES "chain/*.cpp")
file(GLOB INDEX_HEADERS "index/*.hpp")
file(GLOB INDEX_SOURCES "index/*.cpp")
file(GLOB UTIL_HEADERS "util/*.hpp")
file(GLOB UTIL_SOURCES "util/*.cpp")
file(GLOB HEURISTICS_HEADERS "heuristics/*.hpp")
file(GLOB HEURISTICS_SOURCES "heuristics/*.cpp")

set(ALL_SOURCES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES} ${ADDRESS_HEADERS} ${ADDRESS_SOURCES}  ${SCRIPT_HEADERS} ${SCRIPT_SOURCES} ${CHAIN_HEADERS} ${CHAIN_SOURCES} ${INDEX_HEADERS} ${INDEX_SOURCES} ${UTIL_HEADERS} ${UTIL_SOURCES} ${HEURISTICS_HEADERS} ${HEURISTICS_SOURCES})

add_library(blocksci SHARED ${ALL_SOURCES})

if( supported )
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET blocksci PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()

source_group(blocksci\\chain FILES ${CHAIN_HEADERS} ${CHAIN_SOURCES})
source_group(blocksci\\address FILES ${ADDRESS_HEADERS} ${ADDRESS_SOURCES})
source_group(blocksci\\scripts FILES ${SCRIPT_HEADERS} ${SCRIPT_SOURCES})
source_group(blocksci\\index FILES ${INDEX_HEADERS} ${INDEX_SOURCES})
source_group(blocksci\\util FILES ${UTIL_HEADERS} ${UTIL_SOURCES})
source_group(blocksci\\heuristics FILES ${HEURISTICS_HEADERS} ${HEURISTICS_SOURCES})
source_group(blocksci FILES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES})

target_link_libraries( blocksci Threads::Threads)
target_link_libraries( blocksci boost )
target_link_libraries( blocksci secp256k1)
target_link_libraries( blocksci openssl )
target_link_libraries( blocksci lmdbxx )
target_link_libraries (blocksci mpark_variant Ranges)


target_include_directories(blocksci PUBLIC BEFORE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/blocksci>
)

install(TARGETS blocksci DESTINATION lib )
install(FILES ${BLOCKSCI_HEADERS} DESTINATION include/blocksci)
install(FILES ${ADDRESS_HEADERS} DESTINATION include/blocksci/address)
install(FILES ${SCRIPT_HEADERS} DESTINATION include/blocksci/scripts)
install(FILES ${UTIL_HEADERS} DESTINATION include/blocksci/util)
install(FILES ${INDEX_SOURCES} DESTINATION include/blocksci/index)
install(FILES ${CHAIN_HEADERS} DESTINATION include/blocksci/chain)
install(FILES ${HEURISTICS_HEADERS} DESTINATION include/blocksci/heuristics)

