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

#include <boost/filesystem/path.hpp>

#include <stdio.h>

class Cluster;

template<blocksci::EquivAddressType::Enum type>
struct ScriptClusterIndexFile : public blocksci::FixedSizeFileMapper<uint32_t> {
    using blocksci::FixedSizeFileMapper<uint32_t>::FixedSizeFileMapper;
};

class ClusterExpander;

struct TaggedCluster {
    Cluster cluster;
    std::vector<TaggedEquivAddress> taggedEquivAddresses;
    
    TaggedCluster(const Cluster &cluster_, std::vector<TaggedEquivAddress> &&taggedEquivAddresses_) : cluster(cluster_), taggedEquivAddresses(taggedEquivAddresses_) {}
};

class ClusterManager {
    blocksci::FixedSizeFileMapper<uint32_t> clusterOffsetFile;
    blocksci::FixedSizeFileMapper<blocksci::EquivAddress> clusterScriptsFile;
    
    using ScriptClusterIndexTuple = blocksci::to_equiv_address_tuple_t<ScriptClusterIndexFile>;
    
    ScriptClusterIndexTuple scriptClusterIndexFiles;
    
public:
    ClusterManager(const boost::filesystem::path &baseDirectory);
    
    Cluster getCluster(const blocksci::Address &address) const;
    Cluster getCluster(const blocksci::EquivAddress &address) const;
    
    template<blocksci::EquivAddressType::Enum type>
    uint32_t getClusterNum(uint32_t scriptNum) const {
        auto &file = std::get<ScriptClusterIndexFile<type>>(scriptClusterIndexFiles);
        return *file.getData(scriptNum - 1);
    }
    
    uint32_t getClusterNum(const blocksci::EquivAddress &address) const;
    
    uint32_t getClusterSize(uint32_t clusterNum) const;
    boost::iterator_range<const blocksci::EquivAddress *> getClusterScripts(uint32_t clusterNum) const;
    
    uint32_t clusterCount() const;

    auto getClusters() const {
        return ranges::view::ints(0u,clusterCount()) 
        | ranges::view::transform([&](uint32_t clusterNum) { return Cluster(clusterNum, *this); });
    }
    
    std::vector<uint32_t> getClusterSizes() const;
    
    std::vector<TaggedCluster> taggedClusters(const std::unordered_map<blocksci::EquivAddress, std::string> &tags);
};

using cluster_range = decltype(std::declval<ClusterManager>().getClusters());

#endif /* cluster_manager_hpp */
