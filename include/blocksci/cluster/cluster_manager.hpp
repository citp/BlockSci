//
//  cluster_manager.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef cluster_manager_hpp
#define cluster_manager_hpp

#include "cluster_fwd.hpp"
#include "cluster.hpp"

#include <blocksci/blocksci_export.h>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/iota.hpp>

namespace blocksci {
    namespace heuristics {
        struct ChangeHeuristic;
    }

    class BLOCKSCI_EXPORT ClusterManager {
        ClusterAccess access;
        
        friend class blocksci::Cluster;
        
    public:
        ClusterManager(const std::string &baseDirectory, DataAccess &access);
        
        static ClusterManager createClustering(Blockchain &chain, const heuristics::ChangeHeuristic &heuristic, const std::string &outputPath, bool overwrite = false, bool ignoreCoinjoin = true);
        
        Cluster getCluster(const Address &address) const;
        
        auto getClusters() const {
            return ranges::view::ints(0u, access.clusterCount())
            | ranges::view::transform([&](uint32_t clusterNum) { return Cluster(clusterNum, access); });
        }
        
        auto taggedClusters(const std::unordered_map<Address, std::string> &tags) {
            return getClusters() | ranges::view::transform([tags](Cluster && cluster) -> ranges::optional<TaggedCluster> {
                return cluster.getTaggedUnsafe(tags);
            }) | flatMapOptionals();
        }
    };
    
    using cluster_range = decltype(std::declval<ClusterManager>().getClusters());
} // namespace blocksci


#endif /* cluster_manager_hpp */
