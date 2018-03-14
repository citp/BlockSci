cmake_minimum_required(VERSION 2.8.9)
project(blocksci_cluster)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
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

find_package( Boost 1.58 COMPONENTS program_options system filesystem thread iostreams REQUIRED )

include_directories( ${Boost_INCLUDE_DIR} )
include_directories(./src)
include_directories(../src)
include_directories(../libs/range-v3/include)

link_directories(${Boost_LIBRARY_DIR})
link_directories(/usr/local/lib)
link_directories(.)

add_library(sparsepp INTERFACE)
target_include_directories(sparsepp INTERFACE ../libs/sparsepp)

add_library(dset INTERFACE)
target_include_directories(dset INTERFACE ../libs)

add_subdirectory(../libs/pybind11 ${CMAKE_CURRENT_BINARY_DIR}/pybind11)

add_subdirectory(src/libcluster)
add_subdirectory(src/clusterer)
add_subdirectory(src/python_cluster)
add_subdirectory(src/example)
