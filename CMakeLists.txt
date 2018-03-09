cmake_minimum_required(VERSION 3.9)
project(blocksci)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MACOSX_RPATH 1)

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

find_package( Boost 1.58 COMPONENTS system filesystem thread iostreams serialization REQUIRED )
add_library(boost INTERFACE)
target_link_libraries(boost INTERFACE ${Boost_LIBRARIES})
target_include_directories(boost INTERFACE SYSTEM ${Boost_INCLUDE_DIR})

find_package(OpenSSL REQUIRED)
add_library(openssl INTERFACE)
target_link_libraries(openssl INTERFACE ${OPENSSL_LIBRARIES})
target_include_directories(openssl INTERFACE SYSTEM ${OPENSSL_INCLUDE_DIR})

add_library(Ranges INTERFACE)
target_include_directories(Ranges INTERFACE SYSTEM libs/range-v3/include)

add_library(clipp INTERFACE)
target_include_directories(clipp INTERFACE SYSTEM libs/clipp/include)

link_directories(/usr/local/lib)
include_directories("src")

set(PYBIND11_INSTALL true)
add_subdirectory(libs/pybind11)

add_subdirectory(libs/variant)

add_subdirectory(libs/type_safe)

add_subdirectory(src/blocksci)
add_subdirectory(src/parser)
add_subdirectory(src/mempool_recorder)
add_subdirectory(src/python-interface)
add_subdirectory(src/example)
