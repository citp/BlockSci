//
//  cluster_manager.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef cluster_manager_hpp
#define cluster_manager_hpp

#include "cluster.hpp"

#include <blocksci/util/file_mapper.hpp>
#include <blocksci/script.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/scripts/script_info.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/iota.hpp>

#include <stdio.h>

class Cluster;

template<auto type>
struct ScriptClusterIndexFile : public blocksci::FixedSizeFileMapper<uint32_t> {
    using blocksci::FixedSizeFileMapper<uint32_t>::FixedSizeFileMapper;
};

class ClusterExpander;

struct TaggedCluster {
    Cluster cluster;
    std::vector<TaggedScript> taggedScripts;
    
    TaggedCluster(const Cluster &cluster_, std::vector<TaggedScript> &&taggedScripts_) : cluster(cluster_), taggedScripts(taggedScripts_) {}
};

class ClusterManager {
    blocksci::FixedSizeFileMapper<uint32_t> clusterOffsetFile;
    blocksci::FixedSizeFileMapper<blocksci::Script> clusterScriptsFile;
    
    using ScriptClusterIndexTuple = blocksci::to_script_tuple_t<ScriptClusterIndexFile>;
    
    ScriptClusterIndexTuple scriptClusterIndexFiles;
    
public:
    ClusterManager(std::string baseDirectory);
    
    Cluster getCluster(const blocksci::Address &address) const;
    
    template<blocksci::ScriptType::Enum type>
    uint32_t getClusterNum(uint32_t addressNum) const {
        auto &file = std::get<ScriptClusterIndexFile<type>>(scriptClusterIndexFiles);
        return *file.getData(addressNum);
    }
    
    uint32_t getClusterNum(const blocksci::Address &address) const;
    
    uint32_t getClusterSize(uint32_t clusterNum) const;
    boost::iterator_range<const blocksci::Script *> getClusterScripts(uint32_t clusterNum) const;
    
    uint32_t clusterCount() const;

    auto getClusters() const {
        return ranges::view::ints(0u,clusterCount()) 
        | ranges::view::transform([&](uint32_t clusterNum) { return Cluster(clusterNum, *this); });
    }
    
    std::vector<uint32_t> getClusterSizes() const;
    
    std::vector<TaggedCluster> taggedClusters(const std::unordered_map<blocksci::Script, std::string> &tags);
};

using cluster_range = decltype(std::declval<ClusterManager>().getClusters());

#endif /* cluster_manager_hpp */
