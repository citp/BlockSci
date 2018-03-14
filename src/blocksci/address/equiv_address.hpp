//
//  equiv_address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/13/18.
//

#ifndef equiv_address_hpp
#define equiv_address_hpp

#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/address/address.hpp>

#include <unordered_set>

namespace blocksci {
    class EquivAddress {
        std::unordered_set<Address> possibleAddresses;
        bool validated;
        bool scriptEquivalent;
        const DataAccess &access;
        
        void validateAddresses();
    public:
        
        EquivAddress(std::unordered_set<Address> possibleAddresses_, bool scriptEquivalent_, const DataAccess &access_) : possibleAddresses(std::move(possibleAddresses_)), validated(false), scriptEquivalent(scriptEquivalent_), access(access_) {}
        
        bool operator==(const EquivAddress &other) const {
            if (scriptEquivalent != other.scriptEquivalent) {
                return false;
            }
            auto a = getAddresses();
            auto b = other.getAddresses();
            return a == b;
        }
        
        std::unordered_set<Address> getAddresses() const;
        
        std::unordered_set<Address>::const_iterator begin() {
            validateAddresses();
            return possibleAddresses.begin();
        }
        
        std::unordered_set<Address>::const_iterator end() {
            validateAddresses();
            return possibleAddresses.end();
        }
        
        std::vector<OutputPointer> getOutputPointers() const;
        
        uint64_t calculateBalance(BlockHeight height) const;
        std::vector<Output> getOutputs() const;
        std::vector<Input> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
    };
}

namespace std {
    template <>
    struct hash<blocksci::EquivAddress> {
        typedef blocksci::EquivAddress argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const;
    };
}


#endif /* equiv_address_hpp */
