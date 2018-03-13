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

uint64_t Cluster::calculateBalance(blocksci::BlockHeight height) const {
    uint64_t value = 0;
    if (height == 0) {
        for (auto &output : getOutputs()) {
            if (!output.isSpent()) {
                value += output.getValue();
            }
        }
    } else {
        for (auto &output : getOutputs()) {
            if (output.blockHeight <= height && (!output.isSpent() || output.getSpendingTx()->blockHeight > height)) {
                value += output.getValue();
            }
        }
    }
    return value;
}

std::vector<blocksci::Output> Cluster::getOutputs() const {
    std::unordered_set<blocksci::Output> outputs;
    for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
        for (auto &address : manager.access.addressIndex->getPossibleEquivAddresses(dedupAddress, false, manager.access)) {
            auto addrOuts = address.getOutputs(false, false);
            outputs.insert(addrOuts.begin(), addrOuts.end());
        }
    }
    return std::vector<blocksci::Output>{outputs.begin(), outputs.end()};
}

std::vector<blocksci::Input> Cluster::getInputs() const {
    std::unordered_set<blocksci::Input> outputs;
    for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
        for (auto &address : manager.access.addressIndex->getPossibleEquivAddresses(dedupAddress, false, manager.access)) {
            auto addrOuts = address.getInputs(false, false);
            outputs.insert(addrOuts.begin(), addrOuts.end());
        }
    }
    return std::vector<blocksci::Input>{outputs.begin(), outputs.end()};
}

std::vector<blocksci::Transaction> Cluster::getTransactions() const {
    std::unordered_set<blocksci::Transaction> outputs;
    for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
        for (auto &address : manager.access.addressIndex->getPossibleEquivAddresses(dedupAddress, false, manager.access)) {
            auto addrOuts = address.getTransactions(false, false);
            outputs.insert(addrOuts.begin(), addrOuts.end());
        }
    }
    return std::vector<blocksci::Transaction>{outputs.begin(), outputs.end()};
}

std::vector<blocksci::Transaction> Cluster::getOutputTransactions() const {
    std::unordered_set<blocksci::Transaction> outputs;
    for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
        for (auto &address : manager.access.addressIndex->getPossibleEquivAddresses(dedupAddress, false, manager.access)) {
            auto addrOuts = address.getOutputTransactions(false, false);
            outputs.insert(addrOuts.begin(), addrOuts.end());
        }
    }
    return std::vector<blocksci::Transaction>{outputs.begin(), outputs.end()};
}

std::vector<blocksci::Transaction> Cluster::getInputTransactions() const {
    std::unordered_set<blocksci::Transaction> outputs;
    for (auto &dedupAddress : manager.getClusterScripts(clusterNum)) {
        for (auto &address : manager.access.addressIndex->getPossibleEquivAddresses(dedupAddress, false, manager.access)) {
            auto addrOuts = address.getInputTransactions(false, false);
            outputs.insert(addrOuts.begin(), addrOuts.end());
        }
    }
    return std::vector<blocksci::Transaction>{outputs.begin(), outputs.end()};
}

