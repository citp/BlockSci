//
//  cluster_manager.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#include "cluster_manager.hpp"
#include "cluster.hpp"

#include <blocksci/address/address_info.hpp>
#include <blocksci/address/dedup_address.hpp>

ClusterManager::ClusterManager(const boost::filesystem::path &baseDirectory, const blocksci::DataAccess &access_) : 
    clusterOffsetFile(baseDirectory/"clusterOffsets"), 
    clusterScriptsFile(baseDirectory/"clusterAddresses"), 
    scriptClusterIndexFiles(blocksci::apply(blocksci::DedupAddressInfoList(), [&] (auto tag) {
        std::stringstream ss;
        ss << blocksci::dedupAddressName(tag) << "_cluster_index";
        return baseDirectory/ss.str();
    })),
    access(access_)  {}

uint32_t ClusterManager::clusterCount() const {
    return static_cast<uint32_t>(clusterOffsetFile.size());
}

template<blocksci::DedupAddressType::Enum type>
struct ClusterNumFunctor {
    static uint32_t f(const ClusterManager *cm, uint32_t scriptNum) {
        return cm->getClusterNumImpl<type>(scriptNum);
    }
};


uint32_t ClusterManager::getClusterNum(const blocksci::Address &address) const {
    static auto table = blocksci::make_dynamic_table<blocksci::DedupAddressType, ClusterNumFunctor>();
    static constexpr std::size_t size = blocksci::DedupAddressType::all.size();
    
    auto index = static_cast<size_t>(dedupType(address.type));
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return table[index](this, address.scriptNum);
}

Cluster ClusterManager::getCluster(const blocksci::Address &address) const {
    return Cluster(getClusterNum(address), *this);
}

std::vector<TaggedCluster> ClusterManager::taggedClusters(const std::unordered_map<blocksci::Address, std::string> &tags) {
    std::vector<TaggedCluster> taggedClusters;
    for (auto cluster : getClusters()) {
        auto taggedAddresses = cluster.taggedAddresses(tags);
        if (!taggedAddresses.empty()) {
            taggedClusters.emplace_back(cluster, std::move(taggedAddresses));
        }
    }
    return taggedClusters;
}

uint32_t ClusterManager::getClusterSize(uint32_t clusterNum) const {
    auto clusterOffset = *clusterOffsetFile.getData(clusterNum);
    auto clusterSize = clusterOffset;
    if (clusterNum > 0) {
        clusterSize -= *clusterOffsetFile.getData(clusterNum - 1);
    }
    return clusterSize;
}

std::vector<uint32_t> ClusterManager::getClusterSizes() const {
    auto tot = clusterCount();
    std::vector<uint32_t> clusterSizes;
    clusterSizes.resize(tot);
    
    clusterSizes[tot - 1] = *clusterOffsetFile.getData(tot - 1);
    for (uint32_t i = 2; i <= tot; i++) {
        clusterSizes[tot - i] = *clusterOffsetFile.getData(tot - i);
        clusterSizes[(tot - i) + 1] -= clusterSizes[tot - i];
    }
    return clusterSizes;
}

ranges::iterator_range<const blocksci::DedupAddress *> ClusterManager::getClusterScripts(uint32_t clusterNum) const {
    auto nextClusterOffset = *clusterOffsetFile.getData(clusterNum);
    uint32_t clusterOffset = 0;
    if (clusterNum > 0) {
        clusterOffset = *clusterOffsetFile.getData(clusterNum - 1);
    }
    auto clusterSize = nextClusterOffset - clusterOffset;
    
    auto firstAddressOffset = clusterScriptsFile.getData(clusterOffset);
    
    return ranges::make_iterator_range(firstAddressOffset, firstAddressOffset + clusterSize);
}
