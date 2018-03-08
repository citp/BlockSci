//
//  cluster.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef cluster_hpp
#define cluster_hpp

#include <blocksci/address/equiv_address.hpp>


#include <boost/range/iterator_range.hpp>

#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <cstdint>

class ClusterManager;

struct TaggedEquivAddress {
    blocksci::EquivAddress address;
    std::string tag;
    
    TaggedEquivAddress(const blocksci::EquivAddress &address_, const std::string &tag_) : address(address_), tag(tag_) {}
};

class Cluster {
    const ClusterManager &manager;
    
public:
    uint32_t clusterNum;
    
    Cluster(uint32_t clusterNum_, const ClusterManager &manager_) : manager(manager_), clusterNum(clusterNum_) {}
    boost::iterator_range<const blocksci::EquivAddress *> getEquivAddresses() const;
    
    std::vector<TaggedEquivAddress> taggedEquivAddresses(const std::unordered_map<blocksci::EquivAddress, std::string> &tags) const;
    
    uint32_t countOfType(blocksci::EquivAddressType::Enum type) const;
    
    uint32_t getSize() const;
    
    bool operator==(const Cluster &other) {
        return clusterNum == other.clusterNum;
    }
};

#endif /* cluster_hpp */
