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
#include <blocksci/core/dedup_address.hpp>

#include <internal/address_info.hpp>
#include <internal/cluster_access.hpp>
#include <internal/data_access.hpp>
#include <internal/dedup_address_info.hpp>
#include <internal/script_access.hpp>

#include <range/v3/iterator/operations.hpp>
#include <range/v3/view/join.hpp>

namespace {
    using namespace blocksci;
    
    template<DedupAddressType::Enum type>
    struct DedupAddressTypesRangeFunctor {
        static ranges::subrange<const AddressType::Enum *> f() {
            auto &types = DedupAddressInfo<type>::equivTypes;
            return ranges::subrange<const AddressType::Enum *>{types.begin(), types.end()};
        }
    };
    
    inline ranges::subrange<const AddressType::Enum *> addressTypesRange(DedupAddressType::Enum t) {
        static auto &dedupAddressTypesRangeTable = *[]() {
            auto nameTable = make_static_table<DedupAddressType, DedupAddressTypesRangeFunctor>();
            return new decltype(nameTable){nameTable};
        }();
        auto index = static_cast<size_t>(t);
        return dedupAddressTypesRangeTable.at(index);
    }
}

namespace blocksci {
    
    ranges::subrange<const blocksci::DedupAddress *> Cluster::getDedupAddresses() const {
        return clusterAccess->getClusterScripts(clusterNum);
    }
    
    ranges::transform_view<ranges::subrange<const AddressType::Enum *>, ClusterAddressCreator>
    PossibleAddressesGetter::operator()(const DedupAddress &dedupAddress) const {
        ClusterAddressCreator creator{access, dedupAddress.scriptNum};
        return addressTypesRange(dedupAddress.type) | ranges::view::transform(creator);
    }
    
    auto getPossibleAddressesNested(uint32_t clusterNum, const ClusterAccess *clusterAccess) {
        PossibleAddressesGetter getter{&clusterAccess->access};
        return clusterAccess->getClusterScripts(clusterNum) | ranges::view::transform(getter);
    }
    
    auto getPossibleAddresses(uint32_t clusterNum, const ClusterAccess *clusterAccess) {
        return getPossibleAddressesNested(clusterNum, clusterAccess) | ranges::view::join;
    }
    
    auto getAddressesNested(uint32_t clusterNum, const ClusterAccess *clusterAccess) {
        DataAccess *access_ = &clusterAccess->access;
        return clusterAccess->getClusterScripts(clusterNum) | ranges::view::transform([access_](const DedupAddress &address) {
            auto header = access_->getScripts().getScriptHeader(address.scriptNum, address.type);
            uint32_t scriptNum = address.scriptNum;
            return addressTypesRange(address.type) | ranges::view::filter([header](AddressType::Enum type) {
                return header->seenTopLevel(type);
            }) | ranges::view::transform([access_, scriptNum](AddressType::Enum addressType) {
                return Address(scriptNum, addressType, *access_);
            });
        });
    }
    
    ranges::any_view<Address> Cluster::getAddresses() const {
        DataAccess *access_ = &clusterAccess->access;
        return getDedupAddresses() | ranges::view::transform([access_](const DedupAddress &address) {
            auto header = access_->getScripts().getScriptHeader(address.scriptNum, address.type);
            uint32_t scriptNum = address.scriptNum;
            return addressTypesRange(address.type) | ranges::view::filter([header](AddressType::Enum type) {
                return header->seenTopLevel(type);
            }) | ranges::view::transform([access_, scriptNum](AddressType::Enum addressType) {
                return Address(scriptNum, addressType, *access_);
            });
        }) | ranges::view::join;
    }
    
    bool Cluster::containsAddress(const Address &address) const {
        return clusterAccess->getClusterNum(address) == clusterNum;
    }
    
//    ranges::any_view<
//    ranges::any_view<Address, ranges::category::random_access | ranges::category::sized>,
//    ranges::category::random_access | ranges::category::sized
//    > Cluster::getAddressesNested() const {
//        DataAccess *access_ = &clusterAccess->access;
//        return getDedupAddresses() | ranges::view::transform([access_](const DedupAddress &address) -> ranges::any_view<Address, ranges::category::random_access | ranges::category::sized> {
//            auto header = access_->getScripts().getScriptHeader(address.scriptNum, address.type);
//            uint32_t scriptNum = address.scriptNum;
//            return addressTypesRange(address.type) | ranges::view::filter([header](AddressType::Enum type) {
//                return header->seenTopLevel(type);
//            }) | ranges::view::transform([access_, scriptNum](AddressType::Enum addressType) {
//                return Address(scriptNum, addressType, *access_);
//            });
//        });
//    }
    
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
            return TaggedCluster{*this, taggedAddressesNested(tags)};
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
            return TaggedCluster{*this, taggedAddressesNested(tags)};
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::any_view<TaggedAddress> Cluster::taggedAddressesUnsafe(const std::unordered_map<blocksci::Address, std::string> &tags) const {
        return getPossibleAddresses(clusterNum, clusterAccess) | ranges::view::transform([&tags](Address && address) -> ranges::optional<TaggedAddress> {
            auto it = tags.find(address);
            if (it != tags.end()) {
                return TaggedAddress{it->first, it->second};
            } else {
                return ranges::nullopt;
            }
        }) | flatMapOptionals();
    }
    
    ranges::any_view<TaggedAddress> Cluster::taggedAddresses(const std::unordered_map<blocksci::Address, std::string> &tags) const {
        return getPossibleAddresses(clusterNum, clusterAccess) | ranges::view::transform([tags](Address && address) -> ranges::optional<TaggedAddress> {
            auto it = tags.find(address);
            if (it != tags.end()) {
                return TaggedAddress{it->first, it->second};
            } else {
                return ranges::nullopt;
            }
        }) | flatMapOptionals();
    }
    
    ranges::transform_view<ranges::transform_view<ranges::subrange<const DedupAddress *>, PossibleAddressesGetter>, AddressRangeTagChecker>
    Cluster::taggedAddressesNested(const std::unordered_map<blocksci::Address, std::string> &tags) const {
        AddressRangeTagChecker tagCheck{tags};
        
        return getPossibleAddressesNested(clusterNum, clusterAccess) | ranges::view::transform(tagCheck);
    }
    
    uint32_t Cluster::countOfType(AddressType::Enum type) const {
        auto dedupSearchType = dedupType(type);
        uint32_t count = 0;
        for (auto &address : getDedupAddresses()) {
            if (address.type == dedupSearchType) {
                auto header = clusterAccess->access.getScripts().getScriptHeader(address.scriptNum, address.type);
                if (header->seenTopLevel(type)) {
                    ++count;
                }
            }
        }
        return count;
    }
    
    ranges::any_view<OutputPointer> Cluster::getOutputPointers() const {
        return getPossibleAddresses(clusterNum, clusterAccess) | ranges::view::transform([](auto && address) { return address.getOutputPointers(); }) | ranges::view::join;
    }

    ranges::any_view<Output> Cluster::getOutputs() const {
        return getPossibleAddresses(clusterNum, clusterAccess) | ranges::view::transform([](auto && address) { return address.getOutputs(); }) | ranges::view::join;
    }
    
    ranges::any_view<blocksci::Input> Cluster::getInputs() const {
        return getPossibleAddresses(clusterNum, clusterAccess) | ranges::view::transform([](auto && address) { return address.getInputs(); }) | ranges::view::join;
    }
    
    std::vector<blocksci::Transaction> Cluster::getTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getTransactions(pointers, clusterAccess->access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getOutputTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getOutputTransactions(pointers, clusterAccess->access);
    }
    
    std::vector<blocksci::Transaction> Cluster::getInputTransactions() const {
        auto pointers = getOutputPointers() | ranges::to_vector;
        return blocksci::getInputTransactions(pointers, clusterAccess->access);
    }
    
    int64_t Cluster::calculateBalance(BlockHeight height) const {
        auto access_ = &clusterAccess->access;
        auto balances = getDedupAddresses() | ranges::view::transform([access_, height](const DedupAddress &dedupAddress) {
            uint32_t scriptNum = dedupAddress.scriptNum;
            auto possibleAddressBalances = addressTypesRange(dedupAddress.type) | ranges::view::transform([&access_, scriptNum, height](AddressType::Enum addressType) {
                return Address(scriptNum, addressType, *access_).calculateBalance(height);
            });
            return ranges::accumulate(possibleAddressBalances, int64_t{0});
        });
        return ranges::accumulate(balances, int64_t{0});
    }
    
    ranges::any_view<TaggedAddress> TaggedCluster::getTaggedAddresses() const {
        return ranges::view::join(taggedAddresses);
    }
}


