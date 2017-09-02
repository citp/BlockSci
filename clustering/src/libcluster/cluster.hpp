//
//  cluster.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef cluster_hpp
#define cluster_hpp

#include <blocksci/blocksci.hpp>

#include <unordered_map>
#include <stdio.h>
#include <cstdint>

class ClusterManager;

struct TaggedAddressPointer {
    blocksci::AddressPointer pointer;
    std::string tag;
    
    TaggedAddressPointer(const blocksci::AddressPointer &pointer_, const std::string &tag_) : pointer(pointer_), tag(tag_) {}
};

class Cluster {
    const ClusterManager &manager;
    
public:
    uint32_t clusterNum;
    
    Cluster(uint32_t clusterNum_, const ClusterManager &manager_) : manager(manager_), clusterNum(clusterNum_) {}
    boost::iterator_range<const blocksci::AddressPointer *> getAddresses() const;
    
    std::vector<TaggedAddressPointer> taggedAddresses(const std::unordered_map<blocksci::AddressPointer, std::string> &tags) const;
    
    uint32_t getAddressCount(blocksci::ScriptType::Enum type) const;
    
    uint32_t getSize() const;
    
    bool operator==(const Cluster &other) {
        return clusterNum == other.clusterNum;
    }
};

#endif /* cluster_hpp */
