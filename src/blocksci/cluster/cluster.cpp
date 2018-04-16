//
//  cluster.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#include "cluster.hpp"
#include "cluster_manager.hpp"

#include <blocksci/address/dedup_address.hpp>
#include <blocksci/address/equiv_address.hpp>
#include <blocksci/address/address_info.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/index/address_index.hpp>

namespace blocksci {
    std::vector<blocksci::Address> Cluster::getAddresses() const {
        std::vector<blocksci::Address> addresses;
        for (auto &dedupAddress : clusterAccess.getClusterScripts(clusterNum)) {
            for (auto &address : blocksci::EquivAddress(dedupAddress, false, clusterAccess.access)) {
                addresses.push_back(address);
            }
        }
        return addresses;
    }
    
    uint32_t Cluster::getSize() const {
        return clusterAccess.getClusterSize(clusterNum);
    }
    
    std::vector<TaggedAddress> Cluster::taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const {
        if (tags.size() == 0) {
            return {};
        }
        auto &access = tags.begin()->first.getAccess();
        std::vector<TaggedAddress> tagged;
        for (auto &dedupAddress : clusterAccess.getClusterScripts(clusterNum)) {
            for (auto &address : blocksci::EquivAddress(dedupAddress, false, access)) {
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
        for (auto &address : clusterAccess.getClusterScripts(clusterNum)) {
            if (address.type == dedupSearchType) {
                auto searchAddress = blocksci::Address{address.scriptNum, type, clusterAccess.access};
                if (clusterAccess.access.addressIndex.checkIfExists(searchAddress)) {
                    ++count;
                }
            }
        }
        return count;
    }
    
    uint64_t Cluster::calculateBalance(blocksci::BlockHeight height) const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::calculateBalance(pointers, height, clusterAccess.access);
    }
    
    std::vector<blocksci::Input> Cluster::getInputs() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getInputs(pointers, clusterAccess.access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getTransactions(pointers, clusterAccess.access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getOutputTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getOutputTransactions(pointers, clusterAccess.access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getInputTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getInputTransactions(pointers, clusterAccess.access);
    }
}


