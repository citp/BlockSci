//
//  cluster.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#ifndef blocksci_cluster_cluster_hpp
#define blocksci_cluster_cluster_hpp

#include <blocksci/blocksci_export.h>

#include "cluster_access.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/address/dedup_address.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/algorithms.hpp>

#include <range/v3/view/join.hpp>

#include <unordered_map>
#include <vector>

namespace blocksci {
    struct BLOCKSCI_EXPORT TaggedAddress {
        blocksci::Address address;
        std::string tag;
        
        TaggedAddress(const blocksci::Address &address_, const std::string &tag_) : address(address_), tag(tag_) {}
    };
    
    class BLOCKSCI_EXPORT Cluster {
        const ClusterAccess &clusterAccess;
        
    public:
        uint32_t clusterNum;
        
        Cluster(uint32_t clusterNum_, const ClusterAccess &access_) : clusterAccess(access_), clusterNum(clusterNum_) {}
        
        std::vector<blocksci::Address> getAddresses() const;
        
        std::vector<TaggedAddress> taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
        uint32_t countOfType(blocksci::AddressType::Enum type) const;
        
        uint32_t getSize() const;
        
        auto getOutputPointers() const {
            auto &access_ = clusterAccess.access;
            auto scripts = clusterAccess.getClusterScripts(clusterNum);
            return scripts | ranges::view::transform([&access_](const DedupAddress &dedupAddress) {
                uint32_t scriptNum = dedupAddress.scriptNum;
                auto types = addressTypes(dedupAddress.type);
                auto outputsNested = types | ranges::view::transform([&access_, scriptNum](AddressType::Enum addressType) {
                    return Address(scriptNum, addressType, access_).getOutputPointers();
                });
                return outputsNested | ranges::view::join;
            }) | ranges::view::join;
        }
        
        auto getOutputs() const {
            return outputs(getOutputPointers(), clusterAccess.access);
        }
        
        int64_t calculateBalance(BlockHeight height) const {
            return balance(height, getOutputs());
        }
        
        std::vector<Input> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
        
        bool operator==(const Cluster &other) {
            return clusterNum == other.clusterNum;
        }
    };
} // namespace blocksci

#endif /* blocksci_cluster_cluster_hpp */
