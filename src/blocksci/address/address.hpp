//
//  address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#ifndef address_hpp
#define address_hpp

#include "address_types.hpp"

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <range/v3/utility/optional.hpp>

#include <functional>
#include <vector>

namespace blocksci {
    class HashIndex;
    class AddressIndex;
    struct DataConfiguration;
    
    struct Address {
        
        uint32_t scriptNum;
        AddressType::Enum type;
        
        Address();
        Address(uint32_t addressNum, AddressType::Enum type);
        
        bool isSpendable() const;
        
        bool operator==(const Address& other) const {
            return type == other.type && scriptNum == other.scriptNum;
        }
        
        bool operator!=(const Address& other) const {
            return !operator==(other);
        }
        
        bool operator==(const Script &other) const;
        bool operator!=(const Script &other) const;
        
        std::string toString() const;
        
        AnyScript getScript(const ScriptAccess &access) const;
        
        uint64_t calculateBalance(BlockHeight height, const AddressIndex &index, const ChainAccess &chain) const;
        
        std::vector<Output> getOutputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Input> getInputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        

        std::string fullType(const ScriptAccess &script) const;

        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON        
        AnyScript getScript() const;
        
        uint64_t calculateBalance(BlockHeight height) const;
        
        std::vector<Output> getOutputs() const;
        std::vector<Input> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;

        std::string fullType() const;
        #endif
    };
    
    size_t addressCount(const ScriptAccess &access);
    
    void visit(const Address &address, const std::function<bool(const Address &)> &visitFunc, const ScriptAccess &access);
    
    ranges::optional<Address> getAddressFromString(const DataConfiguration &config, HashIndex &index, const std::string &addressString);
    
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix, const ScriptAccess &scripts);
    
    // Requires DataAccess
    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    ranges::optional<Address> getAddressFromString(const std::string &addressString);
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix);
    size_t addressCount();
    #endif

    inline std::ostream &operator<<(std::ostream &os, const Address &address) { 
        return os << address.toString();
    }
}

namespace std {
    template <>
    struct hash<blocksci::Address> {
        typedef blocksci::Address argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.scriptNum) << 32) + static_cast<size_t>(b.type);
        }
    };
}

#endif /* address_hpp */
