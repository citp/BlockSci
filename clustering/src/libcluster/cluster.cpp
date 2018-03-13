//
//  cluster.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#include "cluster.hpp"

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/copy.hpp>

#include "cluster_manager.hpp"

boost::iterator_range<const blocksci::EquivAddress *> Cluster::getEquivAddresses() const {
    return manager.getClusterScripts(clusterNum);
}

uint32_t Cluster::getSize() const {
    return manager.getClusterSize(clusterNum);
}

std::vector<TaggedEquivAddress> Cluster::taggedEquivAddresses(const std::unordered_map<blocksci::EquivAddress, std::string> &tags) const {
    std::vector<TaggedEquivAddress> tagged;
    for (auto &address : getEquivAddresses()) {
        auto it = tags.find(address);
        if (it != tags.end()) {
            tagged.emplace_back(it->first, it->second);
        }
    }
    return tagged;
}

uint32_t Cluster::countOfType(blocksci::EquivAddressType::Enum type) const {
    uint32_t count = 0;
    for (auto &address : getEquivAddresses()) {
        if (address.type == type) {
            ++count;
        }
    }
    return count;
}
