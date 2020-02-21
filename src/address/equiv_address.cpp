//
//  equiv_address.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/13/18.
//

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/core/dedup_address.hpp>

#include <internal/address_info.hpp>
#include <internal/dedup_address_info.hpp>
#include <internal/address_index.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>

#include <range/v3/action/sort.hpp>

#include <sstream>

namespace {
    using namespace blocksci;

    /**
     Returns the script-equivalent addresses from a script-hash address.
     Note, this only returns nested addresses of a script-hash address, not the reverse (i.e., it doesn't look up potential wrapping addresses for non-script-hash addresses)
     */
    std::vector<DedupAddress> getScriptNestedEquivalents(const DedupAddress &searchAddress, DataAccess &access) {
        std::vector<DedupAddress> nestedAddressed;
        auto address = searchAddress;
        while (address.type == DedupAddressType::SCRIPTHASH) {
            auto scriptHash = script::ScriptHash{address.scriptNum, access};
            auto wrapped = scriptHash.getWrappedAddress();
            if (wrapped) {
                nestedAddressed.emplace_back(address);
                address = DedupAddress{wrapped->scriptNum, dedupType(wrapped->type)};
            } else {
                break;
            }
        }
        nestedAddressed.emplace_back(address);
        return nestedAddressed;
    }

    /**
     Returns addresses that are equivalent (i.e., use the same public key)
     @param scriptEquivalent whether nested addresses should be returned for script-hash addresses
     */
    std::unordered_set<Address> initAddresses(const DedupAddress &dedup, bool scriptEquivalent, DataAccess &access) {
        std::unordered_set<DedupAddress> equiv;
        equiv.insert(dedup);
        if (scriptEquivalent) {
            std::vector<DedupAddress> nestedEquiv = getScriptNestedEquivalents(dedup, access);
            for (const auto &address : nestedEquiv) {
                for (auto equivType : equivAddressTypes(address.type)) {
                    auto upAddresses = access.getAddressIndex().getPossibleNestedEquivalentUp({address.scriptNum, equivType});
                    equiv.insert(upAddresses.begin(), upAddresses.end());
                }
            }
        }
        
        std::unordered_set<Address> addresses;
        for (const auto &address : equiv) {
            auto header = access.getScripts().getScriptHeader(address.scriptNum, address.type);
            for (auto equivType : equivAddressTypes(address.type)) {
                if (header->seen(equivType)) {
                    addresses.emplace(address.scriptNum, equivType, access);
                }
            }
        }
        
        return addresses;
    }
}

namespace blocksci {
    EquivAddress::EquivAddress(const Address &address, bool scriptEquivalent_) : scriptEquivalent(scriptEquivalent_), access(&address.getAccess()), addresses(initAddresses(DedupAddress{address.scriptNum, dedupType(address.type)}, scriptEquivalent_, address.getAccess())) {}
    
    EquivAddress::EquivAddress(const DedupAddress &address, bool scriptEquivalent_, DataAccess &access_) : scriptEquivalent(scriptEquivalent_), access(&access_), addresses(initAddresses(address, scriptEquivalent_, access_)) {}
    
    std::string EquivAddress::toString() const {
        auto sortedAddresses = addresses | ranges::to_vector;
        sortedAddresses = std::move(sortedAddresses) | ranges::action::sort;
        
        std::stringstream ss;
        ss << "EquivAddress(";
        size_t i = 0;
        for(auto &address : sortedAddresses) {
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
        return blocksci::getTransactions(getOutputPointers() | ranges::to_vector, *access);
    }
    
    std::vector<Transaction> EquivAddress::getOutputTransactions() const {
        return blocksci::getOutputTransactions(getOutputPointers() | ranges::to_vector, *access);
    }
    
    std::vector<Transaction> EquivAddress::getInputTransactions() const {
        return blocksci::getInputTransactions(getOutputPointers() | ranges::to_vector, *access);
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

