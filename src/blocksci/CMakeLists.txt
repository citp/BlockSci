set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")

find_package( Boost 1.58 COMPONENTS system filesystem thread iostreams serialization REQUIRED )
find_package(OpenSSL REQUIRED)

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
add_library(blocksci_static STATIC ${ALL_SOURCES})

set_property(TARGET blocksci PROPERTY VERSION ${Upstream_VERSION})
set_property(TARGET blocksci_static PROPERTY VERSION ${Upstream_VERSION})

set_property(TARGET blocksci_static PROPERTY POSITION_INDEPENDENT_CODE ON)

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
target_link_libraries( blocksci ${Boost_LIBRARIES})
target_link_libraries( blocksci secp256k1)
target_link_libraries( blocksci OpenSSL::Crypto )
target_link_libraries( blocksci rocksdb )
target_link_libraries( blocksci blocksci_mpark_variant blocksci_ranges)

SET_TARGET_PROPERTIES(blocksci_static PROPERTIES OUTPUT_NAME blocksci)

TARGET_LINK_LIBRARIES(blocksci_static
                        Threads::Threads
                        ${Boost_LIBRARIES}
                        secp256k1
                        OpenSSL::Crypto
                        rocksdb
                        blocksci_mpark_variant blocksci_ranges
                        )



target_include_directories(blocksci INTERFACE ${Boost_INCLUDE_DIR})
target_include_directories(blocksci_static INTERFACE ${Boost_INCLUDE_DIR})

target_include_directories(blocksci PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/blocksci>
)

target_include_directories(blocksci_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/blocksci>
)

SET(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

SET_TARGET_PROPERTIES(blocksci blocksci_static PROPERTIES
    VERSION "${VERSION_STRING}"
    SOVERSION "${SO_VERSION}")

set(INCLUDE_INSTALL_DIR include/blocksci )
set(LIB_INSTALL_DIR lib/ )
set(SYSCONFIG_INSTALL_DIR etc/blocksci/ . )

include(CMakePackageConfigHelpers)

INSTALL(TARGETS blocksci EXPORT blocksci-targets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(TARGETS blocksci_static EXPORT blocksci_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

export(EXPORT blocksci-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/blocksci-targets.cmake"
)

set(ConfigPackageLocation "${LIB_INSTALL_DIR}/cmake/blocksci")

configure_package_config_file(blocksci-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/blocksci-config.cmake
  INSTALL_DESTINATION "${ConfigPackageLocation}"
  PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)

#install (EXPORT  DESTINATION ${CMAKE_CONFIG_DEST}/blocksci-config.cmake)
install (EXPORT blocksci-targets FILE blocksci-targets.cmake DESTINATION ${ConfigPackageLocation} )

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/blocksci-config.cmake"
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT
    Devel
)

install(FILES ${BLOCKSCI_HEADERS} DESTINATION include/blocksci)
install(FILES ${ADDRESS_HEADERS} DESTINATION include/blocksci/address)
install(FILES ${SCRIPT_HEADERS} DESTINATION include/blocksci/scripts)
install(FILES ${UTIL_HEADERS} DESTINATION include/blocksci/util)
install(FILES ${INDEX_HEADERS} DESTINATION include/blocksci/index)
install(FILES ${CHAIN_HEADERS} DESTINATION include/blocksci/chain)
install(FILES ${HEURISTICS_HEADERS} DESTINATION include/blocksci/heuristics)
