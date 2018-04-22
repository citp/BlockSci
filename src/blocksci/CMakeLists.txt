set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")

find_package( Boost 1.58 COMPONENTS filesystem iostreams REQUIRED )
find_package( OpenSSL REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(INCLUDE_INSTALL_DIR include/blocksci )
set(LIB_INSTALL_DIR lib/ )
set(SYSCONFIG_INSTALL_DIR etc/blocksci/ . )

set(BLOCKSCI_HEADER_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/include/blocksci)
set(BLOCKSCI_SOURCE_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(blocksci SHARED "")

set_target_properties(blocksci PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(blocksci PROPERTIES VISIBILITY_INLINES_HIDDEN 1)

target_compile_features(blocksci PUBLIC 
  cxx_aggregate_default_initializers
  cxx_auto_type
  cxx_decltype
  cxx_default_function_template_args
  cxx_defaulted_functions
  cxx_defaulted_move_initializers
  cxx_delegating_constructors
  cxx_deleted_functions
  cxx_explicit_conversions
  cxx_extern_templates
  cxx_generalized_initializers
  cxx_generic_lambdas
  cxx_inheriting_constructors
  cxx_lambdas
  cxx_long_long_type
  cxx_nullptr
  cxx_override
  cxx_range_for
  cxx_return_type_deduction
  cxx_right_angle_brackets
  cxx_rvalue_references
  cxx_static_assert
  cxx_trailing_return_types
  cxx_uniform_initialization
  cxx_variable_templates
  cxx_variadic_templates
  cxx_template_template_parameters
)

target_compile_options(blocksci PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(blocksci PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-covered-switch-default -Wno-missing-prototypes -Wno-weak-vtables -Wno-unused-macros -Wno-padded)
endif()

target_link_libraries( blocksci PUBLIC Threads::Threads )
target_link_libraries( blocksci PRIVATE Boost::filesystem )
target_link_libraries( blocksci PRIVATE Boost::iostreams )
target_link_libraries( blocksci PRIVATE OpenSSL::Crypto )
target_link_libraries( blocksci PRIVATE rocksdb rocksdb_headers)
target_link_libraries( blocksci PRIVATE secp256k1)
target_link_libraries( blocksci PRIVATE dset)
target_link_libraries( blocksci PUBLIC blocksci_mpark_variant blocksci_ranges)

target_include_directories(blocksci PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_include_directories(blocksci PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

set(BLOCKSCI_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/address.hpp
  ${BLOCKSCI_HEADER_PREFIX}/blocksci_fwd.hpp
  ${BLOCKSCI_HEADER_PREFIX}/blocksci.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core.hpp
  ${BLOCKSCI_HEADER_PREFIX}/heuristics.hpp
  ${BLOCKSCI_HEADER_PREFIX}/script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/cluster.hpp
  ${BLOCKSCI_HEADER_PREFIX}/index.hpp
  ${BLOCKSCI_HEADER_PREFIX}/exception.hpp
)

set(ADDRESS_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/address/address_fwd.hpp
  ${BLOCKSCI_HEADER_PREFIX}/address/address_info.hpp
  ${BLOCKSCI_HEADER_PREFIX}/address/address.hpp
  ${BLOCKSCI_HEADER_PREFIX}/address/dedup_address_info.hpp
  ${BLOCKSCI_HEADER_PREFIX}/address/dedup_address.hpp
  ${BLOCKSCI_HEADER_PREFIX}/address/equiv_address_type.hpp
  ${BLOCKSCI_HEADER_PREFIX}/address/equiv_address.hpp
)

set(CHAIN_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/chain/chain_fwd.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/algorithms.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/block.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/blockchain.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/inout_pointer.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/input.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/output.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/transaction_range.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/transaction_summary.hpp
  ${BLOCKSCI_HEADER_PREFIX}/chain/transaction.hpp
)

set(HEURISTICS_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/heuristics/blockchain_heuristics.hpp
  ${BLOCKSCI_HEADER_PREFIX}/heuristics/change_address.hpp
  ${BLOCKSCI_HEADER_PREFIX}/heuristics/taint.hpp
  ${BLOCKSCI_HEADER_PREFIX}/heuristics/tx_identification.hpp
)

set(CLUSTER_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/cluster/cluster_fwd.hpp
  ${BLOCKSCI_HEADER_PREFIX}/cluster/cluster_manager.hpp
  ${BLOCKSCI_HEADER_PREFIX}/cluster/cluster.hpp
  ${BLOCKSCI_HEADER_PREFIX}/cluster/cluster_access.hpp
)

set(INDEX_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/index/address_index.hpp
  ${BLOCKSCI_HEADER_PREFIX}/index/hash_index.hpp
  ${BLOCKSCI_HEADER_PREFIX}/index/mempool_index.hpp
  ${BLOCKSCI_HEADER_PREFIX}/index/column_iterator.hpp
)

set(SCRIPT_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/scripts/scripts_fwd.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/bitcoin_pubkey.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/bitcoin_script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/multisig_script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/multisig_pubkey_script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/nonstandard_script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/nulldata_script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/pubkey_script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/pubkey_base_script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/script_variant.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/script_view.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/script.hpp
  ${BLOCKSCI_HEADER_PREFIX}/scripts/scripthash_script.hpp
)

set(UTIL_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/util/data_access.hpp
  ${BLOCKSCI_HEADER_PREFIX}/util/data_configuration.hpp
  ${BLOCKSCI_HEADER_PREFIX}/util/hash.hpp
  ${BLOCKSCI_HEADER_PREFIX}/util/parallel.hpp
  ${BLOCKSCI_HEADER_PREFIX}/util/progress_bar.hpp
  ${BLOCKSCI_HEADER_PREFIX}/util/state.hpp
  ${BLOCKSCI_HEADER_PREFIX}/util/util.hpp
  ${BLOCKSCI_HEADER_PREFIX}/util/memory_view.hpp
)

set(META_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/meta/apply_template.hpp
  ${BLOCKSCI_HEADER_PREFIX}/meta/apply.hpp
  ${BLOCKSCI_HEADER_PREFIX}/meta/dynamic_table.hpp
  ${BLOCKSCI_HEADER_PREFIX}/meta/static_table.hpp
)

set(CORE_HEADERS
  ${BLOCKSCI_HEADER_PREFIX}/core/address_types.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/dedup_address_type.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/bitcoin_uint256.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/file_mapper.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/hash_combine.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/inout.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/in_place_array.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/chain_access.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/raw_address.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/raw_block.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/raw_transaction.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/script_access.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/script_data.hpp
  ${BLOCKSCI_HEADER_PREFIX}/core/script_info.hpp
)

target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${BLOCKSCI_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${ADDRESS_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${CHAIN_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${SCRIPT_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${HEURISTICS_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${CLUSTER_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${INDEX_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${UTIL_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${META_HEADERS}>)
target_sources(blocksci PUBLIC $<BUILD_INTERFACE:${CORE_HEADERS}>)

set(BLOCKSCI_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/exception.cpp
)

set(ADDRESS_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/address/address_info.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/address/address.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/address/dedup_address_info.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/address/equiv_address.cpp
)

set(CHAIN_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/chain/block.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/chain/blockchain.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/chain/inout_pointer.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/chain/input.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/chain/output.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/chain/transaction.cpp
)

set(HEURISTICS_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/heuristics/blockchain_heuristics.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/heuristics/change_address.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/heuristics/taint.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/heuristics/tx_identification.cpp
)

set(INDEX_PRIVATE_HEADERS
  ${BLOCKSCI_SOURCE_PREFIX}/index/address_index_priv.hpp
  ${BLOCKSCI_SOURCE_PREFIX}/index/hash_index_priv.hpp
)

set(INDEX_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/index/address_index.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/index/address_index_priv.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/index/hash_index.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/index/hash_index_priv.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/index/column_iterator.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/index/mempool_index.cpp
)

set(SCRIPT_PRIVATE_HEADERS
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_base58.hpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_bech32.hpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_segwit_addr.hpp
)

set(SCRIPT_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_base58.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_bech32.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_pubkey.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_script.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_segwit_addr.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/multisig_pubkey_script.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/pubkey_script.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/script_view.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/script.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/script_variant.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/scripts/scripthash_script.cpp
)

set(CORE_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/core/bitcoin_uint256.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/core/chain_access.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/core/file_mapper.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/core/script_access.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/core/script_data.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/core/raw_address.cpp
)

set_source_files_properties(${BLOCKSCI_SOURCE_PREFIX}/scripts/bitcoin_bech32.cpp PROPERTIES COMPILE_FLAGS -Wno-everything)

set(UTIL_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/util/data_configuration.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/util/hash.cpp
)

set_source_files_properties(${BLOCKSCI_SOURCE_PREFIX}/cluster/cluster_manager.cpp PROPERTIES COMPILE_FLAGS "-Wno-reserved-id-macro -Wno-shorten-64-to-32")

set(CLUSTER_SOURCES
  ${BLOCKSCI_SOURCE_PREFIX}/cluster/cluster_manager.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/cluster/cluster.cpp
  ${BLOCKSCI_SOURCE_PREFIX}/cluster/cluster_access.cpp
)

target_sources(blocksci PRIVATE ${BLOCKSCI_SOURCES})
target_sources(blocksci PRIVATE ${ADDRESS_SOURCES})
target_sources(blocksci PRIVATE ${SCRIPT_SOURCES})
target_sources(blocksci PRIVATE ${CHAIN_SOURCES})
target_sources(blocksci PRIVATE ${HEURISTICS_SOURCES})
target_sources(blocksci PRIVATE ${INDEX_SOURCES})
target_sources(blocksci PRIVATE ${UTIL_SOURCES})
target_sources(blocksci PRIVATE ${CLUSTER_SOURCES})
target_sources(blocksci PRIVATE ${INDEX_PRIVATE_HEADERS})
target_sources(blocksci PRIVATE ${SCRIPT_PRIVATE_HEADERS})
target_sources(blocksci PRIVATE ${CORE_SOURCES})

include(GenerateExportHeader)
generate_export_header(blocksci)

set_property(TARGET blocksci PROPERTY VERSION ${Upstream_VERSION})

if( supported )
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET blocksci PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()

source_group(blocksci\\core FILES ${CORE_HEADERS} ${CORE_SOURCES})
source_group(blocksci\\meta FILES ${META_HEADERS} ${META_SOURCES})
source_group(blocksci\\chain FILES ${CHAIN_HEADERS} ${CHAIN_SOURCES})
source_group(blocksci\\address FILES ${ADDRESS_HEADERS} ${ADDRESS_SOURCES})
source_group(blocksci\\scripts FILES ${SCRIPT_HEADERS} ${SCRIPT_SOURCES} ${SCRIPT_PRIVATE_HEADERS})
source_group(blocksci\\index FILES ${INDEX_HEADERS} ${INDEX_PRIVATE_HEADERS} ${INDEX_SOURCES})
source_group(blocksci\\util FILES ${UTIL_HEADERS} ${UTIL_SOURCES})
source_group(blocksci\\heuristics FILES ${HEURISTICS_HEADERS} ${HEURISTICS_SOURCES})
source_group(blocksci\\cluster FILES ${CLUSTER_HEADERS} ${CLUSTER_SOURCES})
source_group(blocksci FILES ${BLOCKSCI_HEADERS} ${BLOCKSCI_SOURCES})

include(CMakePackageConfigHelpers)

INSTALL(TARGETS blocksci EXPORT blocksci-targets LIBRARY DESTINATION ${LIB_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR})

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

install(DIRECTORY ${BLOCKSCI_HEADER_PREFIX}/ DESTINATION ${INCLUDE_INSTALL_DIR})
install(FILES ${BLOCKSCI_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/blocksci_export.h DESTINATION ${INCLUDE_INSTALL_DIR})
