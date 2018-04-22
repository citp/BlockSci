@PACKAGE_INIT@ 

include(CMakeFindDependencyMacro)

find_dependency(Boost 1.58 COMPONENTS system filesystem thread)
find_dependency(Threads)

include("${CMAKE_CURRENT_LIST_DIR}/blocksci-targets.cmake")
