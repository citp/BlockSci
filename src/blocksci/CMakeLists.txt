set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")

find_package( Boost 1.58 COMPONENTS filesystem iostreams serialization REQUIRED )
find_package(OpenSSL REQUIRED)

set(INCLUDE_INSTALL_DIR include/blocksci )
set(LIB_INSTALL_DIR lib/ )
set(SYSCONFIG_INSTALL_DIR etc/blocksci/ . )

set(BLOCKSCI_HEADERS
  address.hpp
  blocksci_fwd.hpp
  blocksci.hpp
  chain.hpp
  heuristics.hpp
  script.hpp
  cluster.hpp
  index.hpp
  exception.hpp
)

set(BLOCKSCI_SOURCES
  exception.cpp
)

set(ADDRESS_HEADERS
  address/address_fwd.hpp
  address/address_info.hpp
  address/address_types.hpp
  address/address.hpp
  address/raw_address.hpp
  address/dedup_address_info.hpp
  address/dedup_address_type.hpp
  address/dedup_address.hpp
  address/equiv_address_type.hpp
  address/equiv_address.hpp
)

set(ADDRESS_SOURCES
  address/address_info.cpp
  address/address.cpp
  address/raw_address.cpp
  address/dedup_address_info.cpp
  address/equiv_address.cpp
)

set(CHAIN_HEADERS
  chain/chain_fwd.hpp
  chain/algorithms.hpp
  chain/block.hpp
  chain/blockchain.hpp
  chain/chain_access.hpp
  chain/inout_pointer.hpp
  chain/inout.hpp
  chain/input.hpp
  chain/output.hpp
  chain/raw_block.hpp
  chain/raw_transaction.hpp
  chain/transaction_range.hpp
  chain/transaction_summary.hpp
  chain/transaction.hpp
)

set(CHAIN_SOURCES
  chain/block.cpp
  chain/blockchain.cpp
  chain/inout_pointer.cpp
  chain/input.cpp
  chain/output.cpp
  chain/transaction.cpp
)

set(HEURISTICS_HEADERS
  heuristics/blockchain_heuristics.hpp
  heuristics/change_address.hpp
  heuristics/taint.hpp
  heuristics/tx_identification.hpp
)

set(HEURISTICS_SOURCES
  heuristics/blockchain_heuristics.cpp
  heuristics/change_address.cpp
  heuristics/taint.cpp
  heuristics/tx_identification.cpp
)

set(INDEX_HEADERS
  index/address_index.hpp
  index/hash_index.hpp
  index/mempool_index.hpp
  index/column_iterator.hpp
)

set(INDEX_PRIVATE_HEADERS
  index/address_index_priv.hpp
  index/hash_index_priv.hpp
)

set(INDEX_SOURCES
  index/address_index.cpp
  index/address_index_priv.cpp
  index/hash_index.cpp
  index/hash_index_priv.cpp
  index/column_iterator.cpp
)

set(SCRIPT_HEADERS
  scripts/scripts_fwd.hpp
  scripts/bitcoin_base58.hpp
  scripts/bitcoin_bech32.hpp
  scripts/bitcoin_pubkey.hpp
  scripts/bitcoin_script.hpp
  scripts/bitcoin_segwit_addr.hpp
  scripts/multisig_script.hpp
  scripts/multisig_pubkey_script.hpp
  scripts/nonstandard_script.hpp
  scripts/nulldata_script.hpp
  scripts/pubkey_script.hpp
  scripts/pubkey_base_script.hpp
  scripts/script_access.hpp
  scripts/script_data.hpp
  scripts/script_info.hpp
  scripts/script_variant.hpp
  scripts/script_view.hpp
  scripts/script.hpp
  scripts/scripthash_script.hpp
)

set(SCRIPT_SOURCES
  scripts/bitcoin_base58.cpp
  scripts/bitcoin_bech32.cpp
  scripts/bitcoin_pubkey.cpp
  scripts/bitcoin_script.cpp
  scripts/bitcoin_segwit_addr.cpp
  scripts/script_view.cpp
  scripts/script.cpp
  scripts/script_variant.cpp
  scripts/scripthash_script.cpp
)

set_source_files_properties(scripts/bitcoin_bech32.cpp PROPERTIES COMPILE_FLAGS -Wno-everything)

set(UTIL_HEADERS
  util/file_mapper_fwd.hpp
  util/bitcoin_uint256.hpp
  util/data_access.hpp
  util/data_configuration.hpp
  util/file_mapper.hpp
  util/file_writer.hpp
  util/hash.hpp
  util/parallel.hpp
  util/progress_bar.hpp
  util/state.hpp
  util/util.hpp
  util/in_place_array.hpp
  util/apply_template.hpp
  util/dynamic_table.hpp
  util/static_table.hpp
  util/memory_view.hpp
)

set(UTIL_SOURCES
  util/bitcoin_uint256.cpp
  util/data_configuration.cpp
  util/hash.cpp
)

set(CLUSTER_HEADERS
  cluster/cluster_fwd.hpp
  cluster/cluster_manager.hpp
  cluster/cluster.hpp
)

set_source_files_properties(cluster/cluster_manager.cpp PROPERTIES COMPILE_FLAGS "-Wno-reserved-id-macro -Wno-shorten-64-to-32")

set(CLUSTER_SOURCES
  cluster/cluster_manager.cpp
  cluster/cluster.cpp
)

set(ALL_SOURCES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES} ${ADDRESS_HEADERS} ${ADDRESS_SOURCES}  ${SCRIPT_HEADERS} ${SCRIPT_SOURCES} ${CHAIN_HEADERS} ${CHAIN_SOURCES} ${INDEX_HEADERS} ${INDEX_PRIVATE_HEADERS} ${INDEX_SOURCES} ${UTIL_HEADERS} ${UTIL_SOURCES} ${HEURISTICS_HEADERS} ${HEURISTICS_SOURCES} ${CLUSTER_HEADERS} ${CLUSTER_SOURCES})

add_library(blocksci SHARED ${ALL_SOURCES})
add_library(blocksci_static STATIC ${ALL_SOURCES})

include(GenerateExportHeader)
generate_export_header(blocksci)
set_target_properties(blocksci_static PROPERTIES COMPILE_FLAGS -DBLOCKSCI_STATIC_DEFINE)

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
source_group(blocksci\\cluster FILES ${CLUSTER_HEADERS} ${CLUSTER_SOURCES})
source_group(blocksci FILES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES})

target_link_libraries( blocksci PUBLIC Threads::Threads)
target_link_libraries( blocksci PUBLIC Boost::filesystem Boost::iostreams Boost::serialization)
target_link_libraries( blocksci PRIVATE OpenSSL::Crypto )
target_link_libraries( blocksci PRIVATE rocksdb )
target_link_libraries( blocksci PRIVATE secp256k1)
target_link_libraries( blocksci PRIVATE dset)
target_link_libraries( blocksci PUBLIC blocksci_mpark_variant blocksci_ranges)

SET_TARGET_PROPERTIES(blocksci_static PROPERTIES OUTPUT_NAME blocksci)

target_link_libraries( blocksci_static PUBLIC Threads::Threads)
target_link_libraries( blocksci_static PUBLIC Boost::filesystem Boost::iostreams Boost::serialization)
target_link_libraries( blocksci_static PRIVATE OpenSSL::Crypto )
target_link_libraries( blocksci_static PRIVATE rocksdb )
target_link_libraries( blocksci_static PRIVATE secp256k1)
target_link_libraries( blocksci_static PRIVATE dset)
target_link_libraries( blocksci_static PUBLIC blocksci_mpark_variant blocksci_ranges)

target_include_directories(blocksci PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_include_directories(blocksci_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

SET(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

SET_TARGET_PROPERTIES(blocksci blocksci_static PROPERTIES
    VERSION "${VERSION_STRING}"
    SOVERSION "${SO_VERSION}")

include(CMakePackageConfigHelpers)

INSTALL(TARGETS blocksci EXPORT blocksci-targets LIBRARY DESTINATION ${LIB_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
INSTALL(TARGETS blocksci_static EXPORT blocksci_static LIBRARY DESTINATION ${LIB_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

export(EXPORT blocksci-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/blocksci-targets.cmake"
)

set(ConfigPackageLocation "${LIB_INSTALL_DIR}/cmake/blocksci")

configure_package_config_file(blocksci-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/blocksci-config.cmake
  INSTALL_DESTINATION "${ConfigPackageLocation}"
  PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)

install (EXPORT blocksci-targets FILE blocksci-targets.cmake DESTINATION ${ConfigPackageLocation} )

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/blocksci-config.cmake"
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT
    Devel
)

install(FILES ${BLOCKSCI_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})
install(FILES ${ADDRESS_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/address)
install(FILES ${SCRIPT_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/scripts)
install(FILES ${UTIL_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/util)
install(FILES ${INDEX_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/index)
install(FILES ${CHAIN_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/chain)
install(FILES ${HEURISTICS_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/heuristics)
install(FILES ${CLUSTER_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/cluster)
