@PACKAGE_INIT@ 

include(CMakeFindDependencyMacro)

find_dependency(OpenSSL)
find_dependency(Threads)

include("${CMAKE_CURRENT_LIST_DIR}/blocksci-targets.cmake")