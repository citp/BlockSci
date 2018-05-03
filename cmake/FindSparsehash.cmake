# Try to find Sparsehash headers.
#
# Usage of this module as follows:
#
#     find_package(Sparsehash)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  SPARSEHASH_ROOT_DIR       Set this variable to the root installation of
#                            Sparsehash if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  SPARSEHASH_FOUND               System has Sparsehash
#  SPARSEHASH_INCLUDE_DIRS        The location of Sparsehash headers.

find_path(SPARSEHASH_ROOT_DIR
    NAMES include/google/sparse_hash_map
)

find_path(SPARSEHASH_INCLUDE_DIRS
    NAMES google/sparse_hash_map
    HINTS ${SPARSEHASH_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(sparsehash DEFAULT_MSG
    SPARSEHASH_INCLUDE_DIRS
)

IF (SPARSEHASH_FOUND)
    add_library(sparsehash INTERFACE)
    target_include_directories(sparsehash INTERFACE ${SPARSEHASH_INCLUDE_DIRS})
ENDIF (SPARSEHASH_FOUND)

mark_as_advanced(SPARSEHASH_INCLUDE_DIRS)
