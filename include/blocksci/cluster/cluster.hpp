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
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/range_util.hpp>
#include <blocksci/util/data_access.hpp>

#include <range/v3/view/filter.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/distance.hpp>

#include <unordered_map>
#include <vector>

namespace blocksci {
    
    template<DedupAddressType::Enum type>
    struct DedupAddressTypesRangeFunctor {
        static ranges::iterator_range<const AddressType::Enum *> f() {
            auto &types = DedupAddressInfo<type>::addressTypes;
            return ranges::iterator_range<const AddressType::Enum *>{types.begin(), types.end()};
        }
    };
    
    inline ranges::iterator_range<const AddressType::Enum *> addressTypesRange(DedupAddressType::Enum t) {
        static auto &dedupAddressTypesRangeTable = *[]() {
            auto nameTable = make_static_table<DedupAddressType, DedupAddressTypesRangeFunctor>();
            return new decltype(nameTable){nameTable};
        }();
        auto index = static_cast<size_t>(t);
        dedupTypeCheckThrow(index);
        return dedupAddressTypesRangeTable[index];
    }
    
    struct BLOCKSCI_EXPORT TaggedAddress {
        blocksci::Address address;
        std::string tag;
        
        TaggedAddress(const blocksci::Address &address_, const std::string &tag_) : address(address_), tag(tag_) {}
    };
    
    struct TaggedCluster;
    
    class BLOCKSCI_EXPORT Cluster {
        const ClusterAccess &clusterAccess;
        
        auto getDedupAddresses() const {
            return clusterAccess.getClusterScripts(clusterNum);
        }
        
        auto getPossibleAddresses() const {
            auto &access_ = clusterAccess.access;
            return getDedupAddresses() | ranges::view::transform([&access_](const DedupAddress &dedupAddress) {
                uint32_t scriptNum = dedupAddress.scriptNum;
                return addressTypesRange(dedupAddress.type) | ranges::view::transform([&access_, scriptNum](AddressType::Enum addressType) {
                    return Address(scriptNum, addressType, access_);
                });
            }) | ranges::view::join;
        }
        
        // Only holds tags by reference so it must remain alive while this range exists
        auto taggedAddressesUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const {
            return getPossibleAddresses() | ranges::view::transform([&tags](Address && address) -> ranges::optional<TaggedAddress> {
                auto it = tags.find(address);
                if (it != tags.end()) {
                    
                    return TaggedAddress{it->first, it->second};
                } else {
                    return ranges::nullopt;
                }
            }) | flatMapOptionals();
        }
        
    public:
        uint32_t clusterNum;
        
        Cluster(uint32_t clusterNum_, const ClusterAccess &access_) : clusterAccess(access_), clusterNum(clusterNum_) {}
        
        ranges::any_view<Address> getAddresses() const;
        
        auto taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const {
            return getPossibleAddresses() | ranges::view::transform([tags](Address && address) -> ranges::optional<TaggedAddress> {
                auto it = tags.find(address);
                if (it != tags.end()) {
                    
                    return TaggedAddress{it->first, it->second};
                } else {
                    return ranges::nullopt;
                }
            }) | flatMapOptionals();
        }
        
        ranges::optional<TaggedCluster> getTaggedUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        ranges::optional<TaggedCluster> getTagged(const std::unordered_map<blocksci::Address, std::string> &tags) const;
        
        uint32_t countOfType(blocksci::AddressType::Enum type) const;
        
        int64_t getSize() const {
            return ranges::distance(getAddresses());
        }
        
        int64_t getTypeEquivSize() const {
            return ranges::distance(getDedupAddresses());
        }
        
        auto getOutputPointers() const {
            return getPossibleAddresses() | ranges::view::transform([](auto && address) { return address.getOutputPointers(); }) | ranges::view::join;
        }
        
        int64_t calculateBalance(BlockHeight height) const {
            auto &access_ = clusterAccess.access;
            auto balances = getDedupAddresses() | ranges::view::transform([&access_, height](const DedupAddress &dedupAddress) {
                uint32_t scriptNum = dedupAddress.scriptNum;
                auto possibleAddressBalances = addressTypesRange(dedupAddress.type) | ranges::view::transform([&access_, scriptNum, height](AddressType::Enum addressType) {
                    return Address(scriptNum, addressType, access_).calculateBalance(height);
                });
                return ranges::accumulate(possibleAddressBalances, int64_t{0});
            });
            return ranges::accumulate(balances, int64_t{0});
        }

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
