
add_executable(blocksci_clusterer main.cpp)

target_link_libraries( blocksci_clusterer dset)
target_link_libraries( blocksci_clusterer clipp)
target_link_libraries( blocksci_clusterer blocksci_static)

install(TARGETS blocksci_clusterer DESTINATION bin)
