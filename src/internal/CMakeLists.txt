set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mtune=native")

find_package( OpenSSL REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SYSCONFIG_INSTALL_DIR etc/blocksci/ . )

set(BLOCKSCI_HEADER_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../include/blocksci)
set(BLOCKSCI_SOURCE_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})

add_library(blocksci_internal STATIC "")
set_property(TARGET blocksci_internal PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties(blocksci_internal PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(blocksci_internal PROPERTIES VISIBILITY_INLINES_HIDDEN 1)

set(BLOCKSCI_FEATURE_REQUIREMENTS
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

target_compile_features(blocksci_internal PUBLIC ${BLOCKSCI_FEATURE_REQUIREMENTS})

target_compile_options(blocksci_internal PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(blocksci_internal PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-covered-switch-default -Wno-missing-prototypes -Wno-weak-vtables -Wno-unused-macros -Wno-padded)
endif()

target_link_libraries( blocksci_internal
PUBLIC
  mio
  filesystem
  rocksdb
  rocksdb_headers
  ranges
PRIVATE
  simpleini
  OpenSSL::Crypto
  endian
)

target_include_directories(blocksci_internal PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/..>
)

set(DATA_ACCESS_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/address_info.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/address_index.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/address_output_range.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin_script.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin_uint256_hex.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/script_view.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/chain_access.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cluster_access.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/data_access.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/data_configuration.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dedup_address.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dedup_address_info.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dedup_address_type.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/exception.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/file_mapper.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/hash.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/hash_index.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/memory_view.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mempool_index.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/progress_bar.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/script_access.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/script_info.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/state.hpp
)

set(DATA_ACCESS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/address_info.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/address_index.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/address_output_range.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin_script.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin_uint256_hex.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dedup_address_info.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/exception.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/hash.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/script_view.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/column_iterator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/data_access.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/data_configuration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/hash_index.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/state.cpp
)

set_source_files_properties(${BLOCKSCI_HEADER_PREFIX}/data_access/bitcoin_script.hpp PROPERTIES COMPILE_FLAGS -Wno-everything)
set_source_files_properties(${BLOCKSCI_SOURCE_PREFIX}/data_access/data_configuration.cpp PROPERTIES COMPILE_FLAGS "-Wno-everything -Wno-zero-as-null-pointer-constant")


target_sources(blocksci_internal
PUBLIC
$<BUILD_INTERFACE:${DATA_ACCESS_HEADERS}>
PRIVATE
${DATA_ACCESS_PRIVATE_HEADERS}
${DATA_ACCESS_SOURCES}
)

source_group(internal FILES ${DATA_ACCESS_HEADERS} ${DATA_ACCESS_PRIVATE_HEADERS} ${DATA_ACCESS_SOURCES})
