include(CheckCXXCompilerFlag)

execute_process(COMMAND git submodule update --init --recursive)

set_property(GLOBAL PROPERTY ranges_headers "${CMAKE_CURRENT_SOURCE_DIR}/range-v3/include/")
set_property(GLOBAL PROPERTY variant_headers "${CMAKE_CURRENT_SOURCE_DIR}/variant/include/")

add_library(variant INTERFACE)
target_include_directories(variant INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/variant/include/)

add_library(ranges INTERFACE)
target_include_directories(ranges INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/range-v3/include/)

add_library(filesystem INTERFACE)
target_include_directories(filesystem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/filesystem/include/)

add_library(mio INTERFACE)
target_include_directories(mio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/mio/include/)

add_library(dset INTERFACE)
target_include_directories(dset INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(clipp INTERFACE)
target_include_directories(clipp SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/clipp/include)

add_library(endian INTERFACE)
target_include_directories(endian SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/endian/src/)

add_library(json INTERFACE)
target_include_directories(json SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/json/include/)

add_library(cereal INTERFACE)
target_include_directories(cereal SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cereal/include/)

include(ExternalProject)


# googletest -- https://github.com/google/googletest/tree/master/googletest#incorporating-into-an-existing-cmake-project

configure_file(../cmake/Googletest.cmake googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)
target_include_directories(gtest_main INTERFACE)

# secp256k1

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

set(saved_flags "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility=hidden")

check_cxx_compiler_flag(-Wclass-memaccess HAS_CLASS_MEMACCESS)
if(HAS_CLASS_MEMACCESS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=class-memaccess")
endif()

option(WITH_TESTS "" OFF)
option(WITH_TOOLS "" OFF)
option(WITH_GFLAGS "" OFF)
# Turn jemalloc off to fix undefined reference to `malloc_stats_print'
option(WITH_JEMALLOC "" OFF)
option(WITH_SNAPPY "" OFF)
option(WITH_LZ4 "" ON)
option(WITH_ZLIB "" OFF)
option(WITH_BZ2 "" OFF)
option(WITH_ZSTD "" OFF)


# rocksdb

add_subdirectory(rocksdb EXCLUDE_FROM_ALL)

add_library(rocksdb_headers INTERFACE)
target_include_directories(rocksdb_headers SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb/include)

set(CMAKE_CXX_FLAGS "${saved_flags}")

add_subdirectory(bitcoin-api-cpp EXCLUDE_FROM_ALL)
