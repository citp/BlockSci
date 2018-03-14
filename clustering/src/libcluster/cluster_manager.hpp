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

template<blocksci::DedupAddressType::Enum type>
struct ScriptClusterIndexFile : public blocksci::FixedSizeFileMapper<uint32_t> {
    using blocksci::FixedSizeFileMapper<uint32_t>::FixedSizeFileMapper;
};

template<blocksci::DedupAddressType::Enum type>
struct ClusterNumFunctor;

struct TaggedCluster {
    Cluster cluster;
    std::vector<TaggedAddress> taggedAddresses;
    
    TaggedCluster(const Cluster &cluster_, std::vector<TaggedAddress> &&taggedAddresses_) : cluster(cluster_), taggedAddresses(taggedAddresses_) {}
};

class ClusterManager {
    blocksci::FixedSizeFileMapper<uint32_t> clusterOffsetFile;
    blocksci::FixedSizeFileMapper<blocksci::DedupAddress> clusterScriptsFile;
    
    using ScriptClusterIndexTuple = blocksci::to_dedup_address_tuple_t<ScriptClusterIndexFile>;
    
    ScriptClusterIndexTuple scriptClusterIndexFiles;
    const blocksci::DataAccess &access;
    
    friend class Cluster;

    template<blocksci::DedupAddressType::Enum type>
    friend struct ClusterNumFunctor;

    boost::iterator_range<const blocksci::DedupAddress *> getClusterScripts(uint32_t clusterNum) const;

    template<blocksci::DedupAddressType::Enum type>
    uint32_t getClusterNumImpl(uint32_t scriptNum) const {
        auto &file = std::get<ScriptClusterIndexFile<type>>(scriptClusterIndexFiles);
        return *file.getData(scriptNum - 1);
    }

public:
    ClusterManager(const boost::filesystem::path &baseDirectory, const blocksci::DataAccess &access);
    
    Cluster getCluster(const blocksci::Address &address) const;
    
    uint32_t getClusterNum(const blocksci::Address &address) const;
    uint32_t getClusterSize(uint32_t clusterNum) const;
    uint32_t clusterCount() const;

    auto getClusters() const {
        return ranges::view::ints(0u,clusterCount()) 
        | ranges::view::transform([&](uint32_t clusterNum) { return Cluster(clusterNum, *this); });
    }
    
    std::vector<uint32_t> getClusterSizes() const;
    
    std::vector<TaggedCluster> taggedClusters(const std::unordered_map<blocksci::Address, std::string> &tags);
};

using cluster_range = decltype(std::declval<ClusterManager>().getClusters());

#endif /* cluster_manager_hpp */
