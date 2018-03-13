//
//  cluster.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef cluster_hpp
#define cluster_hpp

#include <blocksci/address/dedup_address.hpp>


#include <boost/range/iterator_range.hpp>

#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <cstdint>

class ClusterManager;

struct TaggedDedupAddress {
    blocksci::DedupAddress address;
    std::string tag;
    
    TaggedDedupAddress(const blocksci::DedupAddress &address_, const std::string &tag_) : address(address_), tag(tag_) {}
};

class Cluster {
    const ClusterManager &manager;
    
public:
    uint32_t clusterNum;
    
    Cluster(uint32_t clusterNum_, const ClusterManager &manager_) : manager(manager_), clusterNum(clusterNum_) {}
    boost::iterator_range<const blocksci::DedupAddress *> getDedupAddresses() const;
    
    std::vector<TaggedDedupAddress> taggedDedupAddresses(const std::unordered_map<blocksci::DedupAddress, std::string> &tags) const;
    
    uint32_t countOfType(blocksci::DedupAddressType::Enum type) const;
    
    uint32_t getSize() const;
    
    bool operator==(const Cluster &other) {
        return clusterNum == other.clusterNum;
    }
};

#endif /* cluster_hpp */
