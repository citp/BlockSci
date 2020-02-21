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
#include <blocksci/chain/range_util.hpp>
#include <blocksci/core/core_fwd.hpp>
#include <blocksci/core/dedup_address.hpp>

#include <range/v3/range_for.hpp>

#include <unordered_map>
#include <vector>

namespace blocksci {
    class ClusterAccess;
    
    struct BLOCKSCI_EXPORT TaggedAddress {
        blocksci::Address address;
        std::string tag;
        
        TaggedAddress(const blocksci::Address &address_, const std::string &tag_) : address(address_), tag(tag_) {}
    };
    
    inline bool BLOCKSCI_EXPORT operator==(const TaggedAddress& a, const TaggedAddress& b) {
        return a.address == b.address;
    }
    
    inline bool BLOCKSCI_EXPORT operator!=(const TaggedAddress& a, const TaggedAddress& b) {
        return a.address != b.address;
    }
    
    struct TaggedCluster;
    
    struct TagChecker {
        std::unordered_map<blocksci::Address, std::string> tags;
        
        ranges::optional<TaggedAddress> operator()(Address && address) const {
            auto it = tags.find(address);
            if (it != tags.end()) {
                return TaggedAddress{it->first, it->second};
            } else {
                return ranges::nullopt;
            }
        }
    };
    
    struct ClusterAddressCreator {
        DataAccess *access;
        uint32_t scriptNum;
        
        auto operator()(AddressType::Enum addressType) const {
            return Address(scriptNum, addressType, *access);
        }
    };
    
    struct PossibleAddressesGetter {
        DataAccess *access;
        
        ranges::transform_view<ranges::subrange<const AddressType::Enum *>, ClusterAddressCreator>
        operator()(const DedupAddress &dedupAddress) const;
    };
    
    struct AddressRangeTagChecker {
        TagChecker tagCheck;
        
        auto operator()(ranges::transform_view<ranges::subrange<const AddressType::Enum *>, ClusterAddressCreator> && rng) const {
            return std::move(rng) | ranges::view::transform(tagCheck)  | flatMapOptionals();
        }
    };
    
    class BLOCKSCI_EXPORT Cluster {
        const ClusterAccess *clusterAccess;
        
        ranges::subrange<const blocksci::DedupAddress *> getDedupAddresses() const;
        
        // Only holds tags by reference so it must remain alive while this range exists
        ranges::any_view<TaggedAddress> taggedAddressesUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
    public:
        uint32_t clusterNum;
        
        Cluster(uint32_t clusterNum_, const ClusterAccess &access_) : clusterAccess(&access_), clusterNum(clusterNum_) {}
        
        ranges::any_view<Address> getAddresses() const;
        bool containsAddress(const Address &address) const;
        
        ranges::any_view<TaggedAddress> taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
        ranges::transform_view<ranges::transform_view<ranges::subrange<const DedupAddress *>, PossibleAddressesGetter>, AddressRangeTagChecker>
        taggedAddressesNested(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
        ranges::optional<TaggedCluster> getTaggedUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        ranges::optional<TaggedCluster> getTagged(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
        uint32_t countOfType(blocksci::AddressType::Enum type) const;
        
        int64_t getSize() const;
        
        int64_t getTypeEquivSize() const;
        
        ranges::any_view<OutputPointer> getOutputPointers() const;
        
        int64_t calculateBalance(BlockHeight height) const;

        ranges::any_view<Output> getOutputs() const;
        ranges::any_view<Input> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
    };
    
    inline bool BLOCKSCI_EXPORT operator==(const Cluster& a, const Cluster& b) {
        return a.clusterNum == b.clusterNum;
    }
    
    inline bool BLOCKSCI_EXPORT operator!=(const Cluster& a, const Cluster& b) {
        return a.clusterNum != b.clusterNum;
    }
    
    struct BLOCKSCI_EXPORT TaggedCluster {
        Cluster cluster;
        using TaggedRange = decltype(std::declval<Cluster>().taggedAddressesNested(std::unordered_map<blocksci::Address, std::string>{}));
        TaggedRange taggedAddresses;

        ranges::any_view<TaggedAddress> getTaggedAddresses() const;
        
    private:
        
        friend Cluster;
        
        TaggedCluster(const Cluster &cluster_, TaggedRange &&taggedAddresses_) : cluster(cluster_), taggedAddresses(std::move(taggedAddresses_)) {}
    };
    
    inline bool BLOCKSCI_EXPORT operator==(const TaggedCluster& a, const TaggedCluster& b) {
        return a.cluster == b.cluster;
    }
    
    inline bool BLOCKSCI_EXPORT operator!=(const TaggedCluster& a, const TaggedCluster& b) {
        return a.cluster != b.cluster;
    }
    
} // namespace blocksci

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::Cluster> {
        size_t operator()(const blocksci::Cluster &cluster) const {
            return static_cast<size_t>(cluster.clusterNum);
        }
    };
    
    template<> struct BLOCKSCI_EXPORT hash<blocksci::TaggedAddress> {
        size_t operator()(const blocksci::TaggedAddress &address) const {
            std::size_t seed = 894732847;
            blocksci::hash_combine(seed, address.address);
            blocksci::hash_combine(seed, address.tag);
            return seed;
        }
    };
    
    template<> struct BLOCKSCI_EXPORT hash<blocksci::TaggedCluster> {
        size_t operator()(const blocksci::TaggedCluster &cluster) const {
            std::size_t seed = 67813489;
            blocksci::hash_combine(seed, cluster.cluster);
            
            RANGES_FOR(auto address, cluster.getTaggedAddresses()) {
                blocksci::hash_combine(seed, address);
            }
            
            return seed;
        }
    };
}

#endif /* blocksci_cluster_cluster_hpp */
