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

namespace blocksci {
    namespace heuristics {
        struct ChangeHeuristic;
    }
    
    class ClusterAccess;

    class BLOCKSCI_EXPORT ClusterManager {
        std::unique_ptr<ClusterAccess> access;
        uint32_t clusterCount;
        
        friend class blocksci::Cluster;
        
    public:
        ClusterManager(const std::string &baseDirectory, DataAccess &access);
        ClusterManager(ClusterManager && other);
        ClusterManager &operator=(ClusterManager && other);
        ~ClusterManager();
        
        static ClusterManager createClustering(BlockRange &chain, const heuristics::ChangeHeuristic &heuristic, const std::string &outputPath, bool overwrite = false);
        static ClusterManager createClustering(BlockRange &chain, const std::function<ranges::optional<Output>(const Transaction &tx)> &changeHeuristic, const std::string &outputPath, bool overwrite);
        
        Cluster getCluster(const Address &address) const;
        
        ranges::any_view<Cluster, ranges::category::random_access | ranges::category::sized> getClusters() const;
        
        ranges::any_view<TaggedCluster> taggedClusters(const std::unordered_map<Address, std::string> &tags) const;
    };
    
    using cluster_range = decltype(std::declval<ClusterManager>().getClusters());
} // namespace blocksci


#endif /* cluster_manager_hpp */
