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

#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>
#include <blocksci/script.hpp>
#include <blocksci/util/file_mapper.hpp>
#include <blocksci/heuristics/change_address.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/iota.hpp>

#include <boost/filesystem/path.hpp>


namespace blocksci {
    template<DedupAddressType::Enum type>
    struct ScriptClusterIndexFile : public FixedSizeFileMapper<uint32_t> {
        using FixedSizeFileMapper<uint32_t>::FixedSizeFileMapper;
    };
    
    template<DedupAddressType::Enum type>
    struct ClusterNumFunctor;
    
    struct TaggedCluster {
        Cluster cluster;
        std::vector<TaggedAddress> taggedAddresses;
        
        TaggedCluster(const Cluster &cluster_, std::vector<TaggedAddress> &&taggedAddresses_) : cluster(cluster_), taggedAddresses(taggedAddresses_) {}
    };
    
    class ClusterManager {
        FixedSizeFileMapper<uint32_t> clusterOffsetFile;
        FixedSizeFileMapper<DedupAddress> clusterScriptsFile;
        
        using ScriptClusterIndexTuple = to_dedup_address_tuple_t<ScriptClusterIndexFile>;
        
        ScriptClusterIndexTuple scriptClusterIndexFiles;
        DataAccess &access;
        
        friend class Cluster;
        
        template<DedupAddressType::Enum type>
        friend struct ClusterNumFunctor;
        
        ranges::iterator_range<const DedupAddress *> getClusterScripts(uint32_t clusterNum) const;
        
        template<DedupAddressType::Enum type>
        uint32_t getClusterNumImpl(uint32_t scriptNum) const {
            auto &file = std::get<ScriptClusterIndexFile<type>>(scriptClusterIndexFiles);
            return *file.getData(scriptNum - 1);
        }
        
    public:
        ClusterManager(const boost::filesystem::path &baseDirectory, DataAccess &access);
        
        static ClusterManager createClustering(Blockchain &chain, const heuristics::ChangeHeuristic &heuristic, const std::string &outputPath, bool overwrite = false);
        
        Cluster getCluster(const Address &address) const;
        
        uint32_t getClusterNum(const Address &address) const;
        uint32_t getClusterSize(uint32_t clusterNum) const;
        uint32_t clusterCount() const;
        
        auto getClusters() const {
            return ranges::view::ints(0u,clusterCount())
            | ranges::view::transform([&](uint32_t clusterNum) { return Cluster(clusterNum, *this); });
        }
        
        std::vector<uint32_t> getClusterSizes() const;
        
        std::vector<TaggedCluster> taggedClusters(const std::unordered_map<Address, std::string> &tags);
    };
    
    using cluster_range = decltype(std::declval<ClusterManager>().getClusters());
} // namespace blocksci


#endif /* cluster_manager_hpp */
