//
//  equiv_address.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/13/18.
//

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/address/dedup_address.hpp>

#include <blocksci/util/data_access.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/algorithms.hpp>

namespace blocksci {
    EquivAddress::EquivAddress(uint32_t scriptNum, EquivAddressType::Enum type, bool scriptEquivalent_, DataAccess &access_) : scriptEquivalent(scriptEquivalent_), access(access_) {
        for (auto equivType : equivAddressTypes(type)) {
            Address address(scriptNum, equivType, access);
            if (scriptEquivalent) {
                auto nested = access.addressIndex.getPossibleNestedEquivalent(address);
                for (auto &nestedAddress : nested) {
                    if (access.addressIndex.checkIfExists(nestedAddress)) {
                        addresses.insert(nestedAddress);
                    }
                }
            } else {
                if (access.addressIndex.checkIfExists(address)) {
                    addresses.insert(address);
                }
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
        const auto &access_ = access;
        return addresses
        | ranges::view::transform([&access_](const Address &address) { return access_.addressIndex.getOutputPointers(address); })
        | ranges::view::join
        ;
    }
    
    int64_t EquivAddress::calculateBalance(BlockHeight height) {
        return balance(height, getOutputs());
    }
    
    ranges::any_view<Output> EquivAddress::getOutputs() {
        return outputs(getOutputPointers(), access);
    }
    
    std::vector<Input> EquivAddress::getInputs() {
        return blocksci::getInputs(getOutputPointers(), access);
    }
    
    std::vector<Transaction> EquivAddress::getTransactions() {
        return blocksci::getTransactions(getOutputPointers(), access);
    }
    
    std::vector<Transaction> EquivAddress::getOutputTransactions() {
        return blocksci::getOutputTransactions(getOutputPointers(), access);
    }
    
    std::vector<Transaction> EquivAddress::getInputTransactions() {
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

