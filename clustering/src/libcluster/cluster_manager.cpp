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

ClusterManager::ClusterManager(std::string baseDirectory) : clusterOffsetFile(baseDirectory + "clusterOffsets"), clusterScriptsFile(baseDirectory + "clusterAddresses"), scriptClusterIndexFiles(blocksci::apply(blocksci::ScriptInfoList(), [&] (auto tag) {
    std::stringstream ss;
    ss << baseDirectory << blocksci::scriptName(tag) << "_cluster_index";
    return ss.str();
}))  {
}

uint32_t ClusterManager::clusterCount() const {
    return static_cast<uint32_t>(clusterOffsetFile.size());
}

template<blocksci::ScriptType::Enum type>
struct ClusterNumFunctor {
    static uint32_t f(const ClusterManager *cm, uint32_t scriptNum) {
        return cm->getClusterNum<type>(scriptNum);
    }
};


uint32_t ClusterManager::getClusterNum(const blocksci::Script &script) const {
    static auto table = blocksci::make_dynamic_table<blocksci::ScriptType, ClusterNumFunctor>();
    static constexpr std::size_t size = blocksci::ScriptType::all.size();
    
    auto index = static_cast<size_t>(script.type);
    if (index >= size)
    {
        throw std::invalid_argument("combination of enum values is not valid");
    }
    return table[index](this, script.scriptNum);
}

Cluster ClusterManager::getCluster(const blocksci::Address &address) const {
    return Cluster(getClusterNum(address), *this);
}

Cluster ClusterManager::getCluster(const blocksci::Script &script) const {
    return Cluster(getClusterNum(address), *this);
}

std::vector<TaggedCluster> ClusterManager::taggedClusters(const std::unordered_map<blocksci::Script, std::string> &tags) {
    std::vector<TaggedCluster> taggedClusters;
    for (auto cluster : getClusters()) {
        auto taggedAddresses = cluster.taggedScripts(tags);
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

boost::iterator_range<const blocksci::Script *> ClusterManager::getClusterScripts(uint32_t clusterNum) const {
    auto nextClusterOffset = *clusterOffsetFile.getData(clusterNum);
    uint32_t clusterOffset = 0;
    if (clusterNum > 0) {
        clusterOffset = *clusterOffsetFile.getData(clusterNum - 1);
    }
    auto clusterSize = nextClusterOffset - clusterOffset;
    
    auto firstAddressOffset = clusterScriptsFile.getData(clusterOffset);
    
    return boost::make_iterator_range_n(firstAddressOffset, clusterSize);
}
