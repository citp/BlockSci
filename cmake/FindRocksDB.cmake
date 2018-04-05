# Try to find RocksDB headers and library.
#
# Usage of this module as follows:
#
#     find_package(RocksDB)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  ROCKSDB_ROOT_DIR          Set this variable to the root installation of
#                            RocksDB if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  ROCKSDB_FOUND               System has RocksDB library/headers.
#  ROCKSDB_LIBRARIES           The RocksDB library.
#  ROCKSDB_INCLUDE_DIRS        The location of RocksDB headers.

find_path(ROCKSDB_ROOT_DIR
    NAMES include/rocksdb/db.h
)

find_library(ROCKSDB_LIBRARIES
    NAMES rocksdb
    HINTS ${ROCKSDB_ROOT_DIR}/lib
)

find_path(ROCKSDB_INCLUDE_DIRS
    NAMES rocksdb/db.h
    HINTS ${ROCKSDB_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RocksDB DEFAULT_MSG
    ROCKSDB_LIBRARIES
    ROCKSDB_INCLUDE_DIRS
)

IF (ROCKSDB_LIBRARIES AND ROCKSDB_INCLUDE_DIRS)
    SET(ROCKSDB_FOUND "YES")
ELSE (ROCKSDB_LIBRARIES AND ROCKSDB_INCLUDE_DIRS)
  SET(ROCKSDB_FOUND "NO")
ENDIF (ROCKSDB_LIBRARIES AND ROCKSDB_INCLUDE_DIRS)

IF (ROCKSDB_FOUND)
    add_library(rocksdb INTERFACE)
    target_link_libraries(rocksdb INTERFACE ${ROCKSDB_LIBRARIES})
    target_include_directories(rocksdb INTERFACE ${ROCKSDB_INCLUDE_DIRS})
ENDIF (ROCKSDB_FOUND)

IF (ROCKSDB_FOUND)
    MESSAGE(STATUS "Found RocksDB: ${ROCKSDB_LIBRARIES}")
ELSE (ROCKSDB_FOUND)
   IF (ROCKSDB_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find RocksDB library include: ${ROCKSDB_INCLUDE_DIRS}, lib: ${ROCKSDB_LIBRARIES}")
   ENDIF (ROCKSDB_FIND_REQUIRED)
ENDIF (ROCKSDB_FOUND)

mark_as_advanced(
    ROCKSDB_ROOT_DIR
    ROCKSDB_LIBRARIES
    ROCKSDB_INCLUDE_DIRS
)
