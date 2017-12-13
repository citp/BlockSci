file(GLOB CLUSTERER_HEADERS "*.hpp")
file(GLOB CLUSTERER_SOURCES "*.cpp")

add_executable(clusterer ${CLUSTERER_SOURCES} ${CLUSTERER_HEADERS})

target_link_libraries( clusterer pthread)
target_link_libraries( clusterer dset)
target_link_libraries( clusterer blocksci)
target_link_libraries( clusterer ${Boost_LIBRARIES})

install(TARGETS clusterer DESTINATION bin)
