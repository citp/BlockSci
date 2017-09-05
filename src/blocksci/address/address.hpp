//
//  address_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#ifndef address_pointer_hpp
#define address_pointer_hpp

#include "address_types.hpp"

#include <boost/optional.hpp>

#include <memory>
#include <vector>

namespace blocksci {
    struct Script;
    struct Transaction;
    struct Output;
    struct Input;
    class ScriptAccess;
    class ChainAccess;
    class AddressFirstSeenAccess;
    class AddressIndex;
    struct DataConfiguration;
    
    struct Address {
        
        uint32_t addressNum;
        AddressType::Enum type;
        
        Address();
        Address(uint32_t addressNum, AddressType::Enum type);
        
        int getDBType() const;
        bool isSpendable() const;
        
        bool operator==(const Address& other) const {
            return type == other.type && addressNum == other.addressNum;
        }
        
        bool operator!=(const Address& other) const {
            return !operator==(other);
        }
        
        std::string toString() const;
        
        Transaction getFirstTransaction(const ChainAccess &chain, const AddressFirstSeenAccess &scriptsFirstSeen) const;
        uint32_t getFirstTransactionIndex(const AddressFirstSeenAccess &access) const;
        
        std::unique_ptr<Script> getScript(const ScriptAccess &access) const;
        
        std::vector<const Output *> getOutputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<const Input *> getInputs(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        std::vector<Transaction> getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const;
        
        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Transaction getFirstTransaction() const;
        uint32_t getFirstTransactionIndex() const;
        
        std::unique_ptr<Script> getScript() const;
        
        std::vector<const Output *> getOutputs() const;
        std::vector<const Input *> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
        #endif
    };
    
    boost::optional<Address> getAddressFromString(const DataConfiguration &config, const ScriptAccess &access, const std::string &addressString);
    
    std::vector<Address> getAddressesFromStrings(const DataConfiguration &config, const ScriptAccess &access, const std::vector<std::string> &addressStrings);
    
    std::vector<Address> getAddressesWithPrefix(const DataConfiguration &config, const ScriptAccess &access, const std::string &prefix);
    size_t addressCount(const ScriptAccess &access);
    
    // Requires DataAccess
    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    boost::optional<Address> getAddressFromString(const std::string &addressString);
    std::vector<Address> getAddressesFromStrings(const std::vector<std::string> &addressStrings);
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix);
    size_t addressCount();
    #endif
}

std::ostream &operator<<(std::ostream &os, const blocksci::Address &address);

namespace std {
    template <>
    struct hash<blocksci::Address> {
        typedef blocksci::Address argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.addressNum) << 32) + static_cast<size_t>(b.type);
        }
    };
}

#endif /* address_pointer_hpp */
