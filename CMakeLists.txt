cmake_minimum_required(VERSION 3.5)
project(blocksci)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

set(CMAKE_MACOSX_RPATH 1)

set(Upstream_VERSION 0.5.0)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

execute_process(COMMAND git submodule update --init)

add_library(dset INTERFACE)
target_include_directories(dset INTERFACE libs)

add_library(blocksci_ranges INTERFACE)
target_include_directories(blocksci_ranges INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/range-v3/include>
  $<INSTALL_INTERFACE:include/blocksci/libs>
)
INSTALL(TARGETS blocksci_ranges EXPORT blocksci-targets)

add_library(blocksci_mpark_variant INTERFACE)
target_include_directories(blocksci_mpark_variant INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libs/variant/include>
  $<INSTALL_INTERFACE:include/blocksci/libs>
)
INSTALL(TARGETS blocksci_mpark_variant EXPORT blocksci-targets)

install(DIRECTORY libs/range-v3/include/ DESTINATION include/blocksci/libs FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY libs/variant/include/ DESTINATION include/blocksci/libs FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY libs/pybind11 DESTINATION ${CMAKE_SOURCE_DIR}/Notebooks/blocksci)

add_library(clipp INTERFACE)
target_include_directories(clipp SYSTEM INTERFACE libs/clipp/include)

include(ExternalProject)
ExternalProject_Add(project_secp256k1
  GIT_REPOSITORY "https://github.com/bitcoin-core/secp256k1.git"
  GIT_TAG "master"
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/secp256k1"
  CONFIGURE_COMMAND CFLAGS=-fPIC CPPFLAGS=-fPIC <SOURCE_DIR>/configure --disable-shared --disable-benchmark --disable-tests --disable-exhaustive-tests --enable-module-recovery --with-bignum=no --prefix <INSTALL_DIR>
  UPDATE_COMMAND "" 
)

ExternalProject_Add_Step(project_secp256k1 autogen
   COMMAND              <SOURCE_DIR>/autogen.sh
   WORKING_DIRECTORY    <SOURCE_DIR>
   DEPENDEES            update
   DEPENDERS            configure
)

ExternalProject_Get_Property(project_secp256k1 install_dir)
add_library(secp256k1 INTERFACE)
add_dependencies(secp256k1 project_secp256k1)
target_include_directories(secp256k1 INTERFACE ${install_dir}/include)
target_link_libraries(secp256k1 INTERFACE ${install_dir}/lib/libsecp256k1.a)

include(cppcheck.cmake)

set(saved_flags "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(WITH_TESTS OFF)
set(WITH_TOOLS OFF)
set(WITH_GFLAGS OFF)
# Turn jemalloc off to fix undefined reference to `malloc_stats_print'
set(WITH_JEMALLOC OFF)
set(WITH_SNAPPY ON)
set(WITH_LZ4 ON)
set(WITH_ZLIB ON)
set(WITH_BZ2 ON)
set(WITH_ZSTD ON)

add_subdirectory(libs/rocksdb EXCLUDE_FROM_ALL)

add_library(rocksdb_headers INTERFACE)
target_include_directories(rocksdb_headers SYSTEM INTERFACE libs/rocksdb/include)

set(CMAKE_CXX_FLAGS "${saved_flags}")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(libs/pybind11)
add_subdirectory(libs/bitcoin-api-cpp EXCLUDE_FROM_ALL)


add_subdirectory(src/blocksci)
add_subdirectory(src/parser)
add_subdirectory(src/mempool_recorder)
add_subdirectory(src/python-interface)
add_subdirectory(src/clusterer)
add_subdirectory(src/benchmark)
add_subdirectory(src/example)

include( CMakePackageConfigHelpers )
set(CONFIG_PACKAGE_INSTALL_DIR lib/cmake/blocksci)

#install(EXPORT blocksci_ranges DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR})
#install(EXPORT blocksci-targets FILE blocksci-config.cmake DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR})
