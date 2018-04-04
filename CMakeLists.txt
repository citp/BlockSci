cmake_minimum_required(VERSION 3.9)
project(blocksci)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MACOSX_RPATH 1)

set(Upstream_VERSION 0.5.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-switch-enum -Wno-missing-prototypes -Wno-weak-vtables -Wno-exit-time-destructors -Wno-unused-macros -Wno-padded -Wno-undefined-func-template")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

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

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

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
target_include_directories(clipp INTERFACE SYSTEM libs/clipp/include)

include(ExternalProject)
ExternalProject_Add(project_secp256k1
  GIT_REPOSITORY "https://github.com/bitcoin-core/secp256k1.git"
  GIT_TAG "master"
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}/secp256k1"
  CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-module-recovery --prefix <INSTALL_DIR>
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
target_link_libraries(secp256k1 INTERFACE ${install_dir}/lib/libsecp256k1.so)
target_link_libraries(secp256k1 INTERFACE gmp)

include(cppcheck.cmake)

add_subdirectory(libs/pybind11)
add_subdirectory(libs/bitcoin-api-cpp)

add_subdirectory(src/blocksci)
add_subdirectory(src/parser)
add_subdirectory(src/mempool_recorder)
add_subdirectory(src/python-interface)
add_subdirectory(src/benchmark)
add_subdirectory(src/example)

include( CMakePackageConfigHelpers )
set(CONFIG_PACKAGE_INSTALL_DIR lib/cmake/blocksci)

#install(EXPORT blocksci_ranges DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR})
#install(EXPORT blocksci-targets FILE blocksci-config.cmake DESTINATION ${CONFIG_PACKAGE_INSTALL_DIR})
