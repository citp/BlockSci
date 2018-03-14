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
        std::unordered_set<Address> addresses;
        bool scriptEquivalent;
        const DataAccess &access;
        
        friend struct std::hash<EquivAddress>;
        
        EquivAddress(uint32_t scriptNum, EquivAddressType::Enum type, bool scriptEquivalent_, const DataAccess &access_);
    public:
        EquivAddress(const Address &address, bool scriptEquivalent);
        EquivAddress(const DedupAddress &address, bool scriptEquivalent, const DataAccess &access);
        
        bool operator==(const EquivAddress &other) const {
            if (scriptEquivalent != other.scriptEquivalent) {
                return false;
            }
            return addresses == other.addresses;
        }
        
        std::unordered_set<Address>::const_iterator begin() const {
            return addresses.begin();
        }
        
        std::unordered_set<Address>::const_iterator end() const {
            return addresses.end();
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
