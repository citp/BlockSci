cmake_minimum_required(VERSION 3.5)
project(blocksci_parser)

find_package(OpenSSL REQUIRED)
find_package(Sparsehash REQUIRED)

find_package( Boost 1.58 COMPONENTS serialization REQUIRED )

file(GLOB PARSER_HEADERS "*.hpp")
file(GLOB PARSER_SOURCES "*.cpp")

set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS "-Wno-reserved-id-macro -Wno-sign-conversion -Wno-extra-semi -Wno-undefined-func-template")

add_executable(blocksci_parser ${PARSER_SOURCES} ${PARSER_HEADERS})

set_target_properties(blocksci_parser PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(blocksci_parser PROPERTIES VISIBILITY_INLINES_HIDDEN 1)

target_compile_options(blocksci_parser PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(blocksci_parser PRIVATE -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-old-style-cast -Wno-documentation-unknown-command -Wno-documentation -Wno-shadow -Wno-covered-switch-default -Wno-missing-prototypes -Wno-weak-vtables -Wno-unused-macros -Wno-padded -Wno-missing-variable-declarations)
endif()

source_group(blocksci_parser FILES ${PARSER_SOURCES} ${PARSER_HEADERS})

target_link_libraries( blocksci_parser OpenSSL::Crypto )
target_link_libraries( blocksci_parser bitcoinapi_static)
target_link_libraries( blocksci_parser clipp)
target_link_libraries( blocksci_parser blocksci_internal)
target_link_libraries( blocksci_parser Boost::serialization)
target_link_libraries( blocksci_parser secp256k1)
target_link_libraries( blocksci_parser sparsehash)
target_link_libraries( blocksci_parser filesystem)
target_link_libraries( blocksci_parser variant)
target_link_libraries( blocksci_parser json)
target_link_libraries( blocksci_parser cereal)


install(TARGETS blocksci_parser DESTINATION bin)
