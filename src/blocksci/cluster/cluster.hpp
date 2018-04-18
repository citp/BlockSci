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
#include <range/v3/distance.hpp>

#include <unordered_map>
#include <vector>

namespace blocksci {
    struct BLOCKSCI_EXPORT TaggedAddress {
        blocksci::Address address;
        std::string tag;
        
        TaggedAddress(const blocksci::Address &address_, const std::string &tag_) : address(address_), tag(tag_) {}
    };
    
    struct TaggedCluster;
    
    class BLOCKSCI_EXPORT Cluster {
        const ClusterAccess &clusterAccess;
        
        auto getPossibleAddresses() const {
            auto &access_ = clusterAccess.access;
            auto scripts = clusterAccess.getClusterScripts(clusterNum);
            return scripts | ranges::view::transform([&access_](const DedupAddress &dedupAddress) {
                uint32_t scriptNum = dedupAddress.scriptNum;
                return addressTypesRange(dedupAddress.type) | ranges::view::transform([&access_, scriptNum](AddressType::Enum addressType) {
                    return Address(scriptNum, addressType, access_);
                });
            }) | ranges::view::join;
        }
        
    public:
        uint32_t clusterNum;
        
        Cluster(uint32_t clusterNum_, const ClusterAccess &access_) : clusterAccess(access_), clusterNum(clusterNum_) {}
        
        auto getAddresses() const {
            auto &addressIndex = clusterAccess.access.addressIndex;
            return getPossibleAddresses() | ranges::view::filter([&addressIndex](const Address &address) {
                return addressIndex.checkIfTopLevel(address);
            });
        }
        
        auto taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const {
            return getAddresses() | ranges::view::transform([tags](Address && address) -> ranges::optional<TaggedAddress> {
                auto it = tags.find(address);
                if (it != tags.end()) {
                    return TaggedAddress{it->first, it->second};
                } else {
                    return ranges::nullopt;
                }
            }) | flatMapOptionals();
        }
        
        ranges::optional<TaggedCluster> getTagged(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
        uint32_t countOfType(blocksci::AddressType::Enum type) const;
        
        int64_t getSize() const {
            return ranges::distance(getAddresses());
        }
        
        auto getOutputPointers() const {
            return getPossibleAddresses() | ranges::view::transform([](auto && address) { return address.getOutputPointers(); }) | ranges::view::join;
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
    
    struct BLOCKSCI_EXPORT TaggedCluster {
        Cluster cluster;
        using TaggedRange = decltype(std::declval<Cluster>().taggedAddresses(std::unordered_map<blocksci::Address, std::string>{}));
        TaggedRange taggedAddresses;
        
    private:
        friend Cluster;
        
        TaggedCluster(const Cluster &cluster_, TaggedRange &&taggedAddresses_) : cluster(cluster_), taggedAddresses(std::move(taggedAddresses_)) {}
    };
    
} // namespace blocksci

#endif /* blocksci_cluster_cluster_hpp */
