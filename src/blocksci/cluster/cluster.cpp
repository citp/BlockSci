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
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/index/address_index.hpp>

namespace blocksci {
    std::vector<blocksci::Address> Cluster::getAddresses() const {
        std::vector<blocksci::Address> addresses;
        for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
            for (auto &address : blocksci::EquivAddress(dedupAddress, false, manager.access)) {
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
        for (auto &address : manager.getClusterScripts(clusterNum)) {
            if (address.type == dedupSearchType) {
                auto searchAddress = blocksci::Address{address.scriptNum, type, manager.access};
                if (manager.access.addressIndex.checkIfExists(searchAddress)) {
                    ++count;
                }
            }
        }
        return count;
    }
    
    std::vector<blocksci::OutputPointer> Cluster::getOutputPointers() const {
        std::vector<blocksci::OutputPointer> pointers;
        for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
            for (auto &address : blocksci::EquivAddress(dedupAddress, false, manager.access)) {
                auto addrOuts = address.getOutputPointers();
                pointers.insert(pointers.end(), addrOuts.begin(), addrOuts.end());
            }
        }
        return pointers;
    }
    
    uint64_t Cluster::calculateBalance(blocksci::BlockHeight height) const {
        return blocksci::calculateBalance(getOutputPointers(), height, manager.access);
    }
    
    std::vector<blocksci::Output> Cluster::getOutputs() const {
        return blocksci::getOutputs(getOutputPointers(), manager.access);
    }
    
    std::vector<blocksci::Input> Cluster::getInputs() const {
        return blocksci::getInputs(getOutputPointers(), manager.access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getTransactions() const {
        return blocksci::getTransactions(getOutputPointers(), manager.access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getOutputTransactions() const {
        return blocksci::getOutputTransactions(getOutputPointers(), manager.access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getInputTransactions() const {
        return blocksci::getInputTransactions(getOutputPointers(), manager.access);
    }
}


