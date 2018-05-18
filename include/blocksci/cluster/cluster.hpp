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

#include <blocksci/address/address.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/core/core_fwd.hpp>

#include <unordered_map>
#include <vector>

namespace blocksci {
    class ClusterAccess;
    
    struct BLOCKSCI_EXPORT TaggedAddress {
        blocksci::Address address;
        std::string tag;
        
        TaggedAddress(const blocksci::Address &address_, const std::string &tag_) : address(address_), tag(tag_) {}
    };
    
    struct TaggedCluster;
    
    class BLOCKSCI_EXPORT Cluster {
        const ClusterAccess &clusterAccess;
        DataAccess *access;
        
        ranges::iterator_range<const blocksci::DedupAddress *> getDedupAddresses() const;
        
        ranges::any_view<Address> getPossibleAddresses() const;
        
        // Only holds tags by reference so it must remain alive while this range exists
        ranges::any_view<TaggedAddress> taggedAddressesUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
    public:
        uint32_t clusterNum;
        
        Cluster(uint32_t clusterNum_, const ClusterAccess &access_) : clusterAccess(access_), clusterNum(clusterNum_) {}
        
        ranges::any_view<Address> getAddresses() const;
        
        ranges::any_view<TaggedAddress> taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
        ranges::optional<TaggedCluster> getTaggedUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        ranges::optional<TaggedCluster> getTagged(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
        uint32_t countOfType(blocksci::AddressType::Enum type) const;
        
        int64_t getSize() const;
        
        int64_t getTypeEquivSize() const;
        
        ranges::any_view<OutputPointer> getOutputPointers() const;
        
        int64_t calculateBalance(BlockHeight height) const;

        ranges::any_view<Output> getOutputs() const;
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
