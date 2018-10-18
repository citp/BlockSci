//
//  equiv_address.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/13/18.
//

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/algorithms.hpp>

#include <internal/address_info.hpp>
#include <internal/dedup_address.hpp>
#include <internal/dedup_address_info.hpp>
#include <internal/address_index.hpp>
#include <internal/data_access.hpp>

#include <sstream>

namespace blocksci {
    EquivAddress::EquivAddress(uint32_t scriptNum, EquivAddressType::Enum type, bool scriptEquivalent_, DataAccess &access_) : scriptEquivalent(scriptEquivalent_), access(access_) {
        for (auto equivType : equivAddressTypes(type)) {
            RawAddress address(scriptNum, equivType);
            if (scriptEquivalent) {
                auto upAddressesRaw = access_.getAddressIndex().getPossibleNestedEquivalentUp(address);
                RANGES_FOR(auto &rawAddress, upAddressesRaw) {
                    addresses.emplace(rawAddress, access_);
                }
                auto downAddresses = getScriptNestedEquivalents(Address{address, access});
                addresses.insert(downAddresses.begin(), downAddresses.end());
            } else {
                addresses.emplace(address, access_);
            }
            
        }
        
        auto end = addresses.end();
        for (auto it = addresses.begin(); it != end;) {
            if (!access_.getAddressIndex().checkIfExists(*it)) {
                it = addresses.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    EquivAddress::EquivAddress(const Address &address, bool scriptEquivalent_) : EquivAddress(address.scriptNum, equivType(address.type), scriptEquivalent_, address.getAccess()) {}
    
    EquivAddress::EquivAddress(const DedupAddress &address, bool scriptEquivalent_, DataAccess &access_) :
    EquivAddress(address.scriptNum, equivType(address.type), scriptEquivalent_, access_) {}
    
    std::string EquivAddress::toString() const {
        std::stringstream ss;
        ss << "EquivAddress(";
        size_t i = 0;
        for (auto &address : addresses) {
            ss << address.toString();
            if (i < addresses.size() - 1) {
                ss << ", ";
            }
            i++;
        }
        ss << ")";
        return ss.str();
    }
    
    ranges::any_view<OutputPointer> EquivAddress::getOutputPointers() const {
        return addresses
        | ranges::view::transform([](const Address &address) { return address.getOutputPointers(); })
        | ranges::view::join
        ;
    }
    
    int64_t EquivAddress::calculateBalance(BlockHeight height) const {
        return balance(height, getOutputs());
    }
    
    ranges::any_view<Output> EquivAddress::getOutputs() const {
        return addresses
        | ranges::view::transform([](const Address &address) { return address.getOutputs(); })
        | ranges::view::join
        ;
    }
    
    ranges::any_view<Input> EquivAddress::getInputs() const {
        return addresses
        | ranges::view::transform([](const Address &address) { return address.getInputs(); })
        | ranges::view::join
        ;
    }
    
    std::vector<Transaction> EquivAddress::getTransactions() const {
        return blocksci::getTransactions(getOutputPointers(), access);
    }
    
    std::vector<Transaction> EquivAddress::getOutputTransactions() const {
        return blocksci::getOutputTransactions(getOutputPointers(), access);
    }
    
    std::vector<Transaction> EquivAddress::getInputTransactions() const {
        return blocksci::getInputTransactions(getOutputPointers(), access);
    }
}

namespace std {
    auto hash<blocksci::EquivAddress>::operator()(const argument_type &equiv) const -> result_type {
        std::size_t seed = 123954;
        for (const auto &address : equiv.addresses) {
            seed ^= address.scriptNum + address.type;
        }
        return seed;
    }
}

