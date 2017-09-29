//
//  cluster.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef cluster_hpp
#define cluster_hpp

#include <blocksci/scripts/script_pointer.hpp>

#include <boost/range/iterator_range.hpp>

#include <vector>
#include <unordered_map>
#include <stdio.h>
#include <cstdint>

class ClusterManager;

namespace blocksci {
    struct ScriptPointer;
}

struct TaggedScriptPointer {
    blocksci::ScriptPointer scriptPointer;
    std::string tag;
    
    TaggedScriptPointer(const blocksci::ScriptPointer &scriptPointer_, const std::string &tag_) : scriptPointer(scriptPointer_), tag(tag_) {}
};

class Cluster {
    const ClusterManager &manager;
    
public:
    uint32_t clusterNum;
    
    Cluster(uint32_t clusterNum_, const ClusterManager &manager_) : manager(manager_), clusterNum(clusterNum_) {}
    boost::iterator_range<const blocksci::ScriptPointer *> getScripts() const;
    
    std::vector<TaggedScriptPointer> taggedScripts(const std::unordered_map<blocksci::ScriptPointer, std::string> &tags) const;
    
    uint32_t getScriptCount(blocksci::ScriptType::Enum type) const;
    
    uint32_t getSize() const;
    
    bool operator==(const Cluster &other) {
        return clusterNum == other.clusterNum;
    }
};

#endif /* cluster_hpp */
