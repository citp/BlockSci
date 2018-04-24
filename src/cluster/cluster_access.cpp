//
//  cluster_unity.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/3/18.
//
//

#include <blocksci/cluster/cluster_access.hpp>

#include <boost/filesystem/path.hpp>

namespace blocksci {
	ClusterAccess::ClusterAccess(const std::string &baseDirectory, DataAccess &access_) :
    clusterOffsetFile((boost::filesystem::path{baseDirectory}/"clusterOffsets").native()),
    clusterScriptsFile((boost::filesystem::path{baseDirectory}/"clusterAddresses").native()),
    scriptClusterIndexFiles(blocksci::apply(DedupAddressType::all(), [&] (auto tag) {
        std::stringstream ss;
        ss << dedupAddressName(tag) << "_cluster_index";
        return (boost::filesystem::path{baseDirectory}/ss.str()).native();
    })),
    access(access_)  {}
} // namespace blocksci

