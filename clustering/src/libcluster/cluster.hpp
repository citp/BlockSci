//
//  cluster.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef cluster_hpp
#define cluster_hpp

#include <blocksci/address/address.hpp>

#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <cstdint>

class ClusterManager;

struct TaggedAddress {
    blocksci::Address address;
    std::string tag;
    
    TaggedAddress(const blocksci::Address &address_, const std::string &tag_) : address(address_), tag(tag_) {}
};

class Cluster {
    const ClusterManager &manager;
    
public:
    uint32_t clusterNum;
    
    Cluster(uint32_t clusterNum_, const ClusterManager &manager_) : manager(manager_), clusterNum(clusterNum_) {}
    std::vector<blocksci::Address> getAddresses() const;
    
    std::vector<TaggedAddress> taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const;
    
    uint32_t countOfType(blocksci::AddressType::Enum type) const;
    
    uint32_t getSize() const;

    std::vector<blocksci::OutputPointer> getOutputPointers() const;
    uint64_t calculateBalance(blocksci::BlockHeight height) const;
    std::vector<blocksci::Output> getOutputs() const;
    std::vector<blocksci::Input> getInputs() const;
    std::vector<blocksci::Transaction> getTransactions() const;
    std::vector<blocksci::Transaction> getOutputTransactions() const;
    std::vector<blocksci::Transaction> getInputTransactions() const;
    
    bool operator==(const Cluster &other) {
        return clusterNum == other.clusterNum;
    }
};

#endif /* cluster_hpp */
