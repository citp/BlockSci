//
//  cluster.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/6/17.
//
//

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/cluster/cluster_manager.hpp>

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/range_util.hpp>

#include <internal/address_info.hpp>
#include <internal/cluster_access.hpp>
#include <internal/data_access.hpp>
#include <internal/dedup_address.hpp>
#include <internal/dedup_address_info.hpp>
#include <internal/address_index.hpp>

#include <range/v3/distance.hpp>
#include <range/v3/view/join.hpp>

namespace {
    using namespace blocksci;
    
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
        return dedupAddressTypesRangeTable.at(index);
    }
}

namespace blocksci {
    
    ranges::iterator_range<const blocksci::DedupAddress *> Cluster::getDedupAddresses() const {
        return clusterAccess.getClusterScripts(clusterNum);
    }
    
    ranges::any_view<Address> Cluster::getPossibleAddresses() const {
        auto access_ = access;
        return getDedupAddresses() | ranges::view::transform([&access_](const DedupAddress &dedupAddress) {
            uint32_t scriptNum = dedupAddress.scriptNum;
            return addressTypesRange(dedupAddress.type) | ranges::view::transform([access_, scriptNum](AddressType::Enum addressType) {
                return Address(scriptNum, addressType, *access_);
            });
        }) | ranges::view::join;
    }
    
    ranges::any_view<Address> Cluster::getAddresses() const {
        auto &addressIndex = clusterAccess.access.getAddressIndex();
        return getPossibleAddresses() | ranges::view::filter([&addressIndex](const Address &address) {
            return addressIndex.checkIfTopLevel(address);
        });
    }
    
    int64_t Cluster::getSize() const {
        return ranges::distance(getAddresses());
    }
    
    int64_t Cluster::getTypeEquivSize() const {
        return ranges::distance(getDedupAddresses());
    }

    ranges::optional<TaggedCluster> Cluster::getTagged(const std::unordered_map<Address, std::string> &tags) const {
        bool isEmpty = [&]() {
            auto addresses = taggedAddresses(tags);
            RANGES_FOR(auto tagged, addresses) {
                return true;
            }
            return false;
        }();
        if (isEmpty) {
            return TaggedCluster{*this, taggedAddresses(tags)};
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::optional<TaggedCluster> Cluster::getTaggedUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const {
        bool isEmpty = [&]() {
            auto addresses = taggedAddressesUnsafe(tags);
            RANGES_FOR(auto tagged, addresses) {
                return true;
            }
            return false;
        }();
        if (isEmpty) {
            return TaggedCluster{*this, taggedAddresses(tags)};
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::any_view<TaggedAddress> Cluster::taggedAddressesUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const {
        return getPossibleAddresses() | ranges::view::transform([&tags](Address && address) -> ranges::optional<TaggedAddress> {
            auto it = tags.find(address);
            if (it != tags.end()) {
                
                return TaggedAddress{it->first, it->second};
            } else {
                return ranges::nullopt;
            }
        }) | flatMapOptionals();
    }
    
    ranges::any_view<TaggedAddress> Cluster::taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const {
        return getPossibleAddresses() | ranges::view::transform([tags](Address && address) -> ranges::optional<TaggedAddress> {
            auto it = tags.find(address);
            if (it != tags.end()) {
                
                return TaggedAddress{it->first, it->second};
            } else {
                return ranges::nullopt;
            }
        }) | flatMapOptionals();
    }
    
    uint32_t Cluster::countOfType(AddressType::Enum type) const {
        auto dedupSearchType = dedupType(type);
        uint32_t count = 0;
        for (auto &address : clusterAccess.getClusterScripts(clusterNum)) {
            if (address.type == dedupSearchType) {
                auto searchAddress = Address{address.scriptNum, type, clusterAccess.access};
                if (clusterAccess.access.getAddressIndex().checkIfTopLevel(searchAddress)) {
                    ++count;
                }
            }
        }
        return count;
    }
    
    ranges::any_view<OutputPointer> Cluster::getOutputPointers() const {
        return getPossibleAddresses() | ranges::view::transform([](auto && address) { return address.getOutputPointers(); }) | ranges::view::join;
    }

    ranges::any_view<Output> Cluster::getOutputs() const {
        return getPossibleAddresses() | ranges::view::transform([](auto && address) { return address.getOutputs(); }) | ranges::view::join;
    }
    
    ranges::any_view<blocksci::Input> Cluster::getInputs() const {
        return getPossibleAddresses() | ranges::view::transform([](auto && address) { return address.getInputs(); }) | ranges::view::join;
    }
    
    std::vector<blocksci::Transaction> Cluster::getTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getTransactions(pointers, clusterAccess.access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getOutputTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getOutputTransactions(pointers, clusterAccess.access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getInputTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getInputTransactions(pointers, clusterAccess.access);
    }
    
    int64_t Cluster::calculateBalance(BlockHeight height) const {
        auto access_ = access;
        auto balances = getDedupAddresses() | ranges::view::transform([access_, height](const DedupAddress &dedupAddress) {
            uint32_t scriptNum = dedupAddress.scriptNum;
            auto possibleAddressBalances = addressTypesRange(dedupAddress.type) | ranges::view::transform([&access_, scriptNum, height](AddressType::Enum addressType) {
                return Address(scriptNum, addressType, *access_).calculateBalance(height);
            });
            return ranges::accumulate(possibleAddressBalances, int64_t{0});
        });
        return ranges::accumulate(balances, int64_t{0});
    }
}


