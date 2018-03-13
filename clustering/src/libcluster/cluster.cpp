//
//  cluster.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#include "cluster.hpp"

#include <blocksci/address/dedup_address.hpp>
#include <blocksci/index/address_index.hpp>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/copy.hpp>

#include "cluster_manager.hpp"

std::vector<blocksci::Address> Cluster::getAddresses() const {
    std::vector<blocksci::Address> addresses;
    for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
        for (auto &address : manager.access.addressIndex->getEquivAddresses(dedupAddress, false, manager.access)) {
            addresses.push_back(address);
        }
    }
    return addresses;
}

uint32_t Cluster::getSize() const {
    return manager.getClusterSize(clusterNum);
}

std::vector<TaggedAddress> Cluster::taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const {
    if (tags.size() == 0) {
        return {};
    }
    auto &access = tags.begin()->first.getAccess();
    std::vector<TaggedAddress> tagged;
    for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
        for (auto &address : access.addressIndex->getPossibleEquivAddresses(dedupAddress, false, access)) {
            auto it = tags.find(address);
            if (it != tags.end()) {
                tagged.emplace_back(it->first, it->second);
            }
        }
    }
    return tagged;
}

uint32_t Cluster::countOfType(blocksci::AddressType::Enum type) const {
    auto dedupSearchType = dedupType(type);
    uint32_t count = 0;
    for (auto &address : manager.getClusterScripts(clusterNum)) {
        if (address.type == dedupSearchType) {
            auto searchAddress = blocksci::Address{address.scriptNum, type, manager.access};
            if (manager.access.addressIndex->checkIfExists(searchAddress)) {
                ++count;
            }
        }
    }
    return count;
}
