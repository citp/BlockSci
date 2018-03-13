//
//  equiv_address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/5/18.
//

#ifndef equiv_address_hpp
#define equiv_address_hpp

#include "address_fwd.hpp"
#include <blocksci/chain/chain_fwd.hpp>

#include <range/v3/utility/optional.hpp>
#include <functional>
#include <vector>

namespace blocksci {
    class HashIndex;
    class AddressIndex;
    struct DataConfiguration;
    
    struct EquivAddress {
        
        uint32_t scriptNum;
        EquivAddressType::Enum type;
        
        EquivAddress();
        EquivAddress(uint32_t addressNum, EquivAddressType::Enum type);
        
        bool operator==(const EquivAddress& other) const {
            return type == other.type && scriptNum == other.scriptNum;
        }
        
        bool operator!=(const EquivAddress& other) const {
            return !operator==(other);
        }
        
        std::string toString() const;
        
        uint64_t calculateBalance(BlockHeight height, const AddressIndex &index, const ChainAccess &chain) const;
        
        std::vector<Output> getOutputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Input> getInputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        
        
        // Requires DataAccess
#ifndef BLOCKSCI_WITHOUT_SINGLETON
        uint64_t calculateBalance(BlockHeight height) const;
        
        std::vector<Output> getOutputs() const;
        std::vector<Input> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
#endif
    };
    
    inline std::ostream &operator<<(std::ostream &os, const EquivAddress &address) {
        return os << address.toString();
    }
}

namespace std {
    template <>
    struct hash<blocksci::EquivAddress> {
        typedef blocksci::EquivAddress argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.scriptNum) << 32) + static_cast<size_t>(b.type);
        }
    };
}

#endif /* equiv_address_hpp */
