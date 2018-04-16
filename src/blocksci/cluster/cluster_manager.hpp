//
//  cluster_manager.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef cluster_manager_hpp
#define cluster_manager_hpp

#include <blocksci/blocksci_export.h>

#include "cluster_fwd.hpp"
#include "cluster.hpp"

#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/script.hpp>
#include <blocksci/util/file_mapper.hpp>
#include <blocksci/heuristics/change_address.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/iota.hpp>

#include <boost/filesystem/path.hpp>


namespace blocksci {
    
    struct BLOCKSCI_EXPORT TaggedCluster {
        Cluster cluster;
        std::vector<TaggedAddress> taggedAddresses;
        
        TaggedCluster(const Cluster &cluster_, std::vector<TaggedAddress> &&taggedAddresses_) : cluster(cluster_), taggedAddresses(taggedAddresses_) {}
    };
    
    class BLOCKSCI_EXPORT ClusterManager {
        ClusterAccess access;
        
        friend class Cluster;
        
    public:
        ClusterManager(const boost::filesystem::path &baseDirectory, DataAccess &access);
        
        static ClusterManager createClustering(Blockchain &chain, const heuristics::ChangeHeuristic &heuristic, const std::string &outputPath, bool overwrite = false);
        
        Cluster getCluster(const Address &address) const;
        
        auto getClusters() const {
            return ranges::view::ints(0u, access.clusterCount())
            | ranges::view::transform([&](uint32_t clusterNum) { return Cluster(clusterNum, access); });
        }
        
        std::vector<TaggedCluster> taggedClusters(const std::unordered_map<Address, std::string> &tags);
    };
    
    using cluster_range = decltype(std::declval<ClusterManager>().getClusters());
} // namespace blocksci


#endif /* cluster_manager_hpp */
