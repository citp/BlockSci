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

#include <boost/range/iterator_range.hpp>

#include <unordered_map>
#include <stdio.h>
#include <cstdint>

class ClusterManager;

namespace blocksci {
    struct Address;
}

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
    boost::iterator_range<const blocksci::Address *> getAddresses() const;
    
    std::vector<TaggedAddress> taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const;
    
    uint32_t getAddressCount(blocksci::AddressType::Enum type) const;
    
    uint32_t getSize() const;
    
    bool operator==(const Cluster &other) {
        return clusterNum == other.clusterNum;
    }
};

#endif /* cluster_hpp */
