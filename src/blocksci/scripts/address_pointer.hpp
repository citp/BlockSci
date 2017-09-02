//
//  address_pointer.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#ifndef address_pointer_hpp
#define address_pointer_hpp

#include "script_types.hpp"

#include <boost/optional.hpp>

#include <memory>
#include <vector>

namespace blocksci {
    struct Address;
    struct Transaction;
    class ScriptAccess;
    class ChainAccess;
    class ScriptFirstSeenAccess;
    struct DataConfiguration;
    
    struct AddressPointer {
        
        uint32_t addressNum;
        ScriptType::Enum type;
        
        AddressPointer();
        AddressPointer(uint32_t addressNum, ScriptType::Enum type);
        
        int getDBType() const;
        bool isSpendable() const;
        
        bool operator==(const AddressPointer& other) const {
            return type == other.type && addressNum == other.addressNum;
        }
        
        bool operator!=(const AddressPointer& other) const {
            return !operator==(other);
        }
        
        std::string toString() const;
        
        Transaction getFirstTransaction(const ChainAccess &chain, const ScriptFirstSeenAccess &scriptsFirstSeen) const;
        uint32_t getFirstTransactionIndex(const ScriptFirstSeenAccess &access) const;
        
        std::unique_ptr<Address> getAddress(const ScriptAccess &access) const;
        
        // Requires DataAccess
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        Transaction getFirstTransaction() const;
        uint32_t getFirstTransactionIndex() const;
        
        std::unique_ptr<Address> getAddress() const;
        #endif
    };
    
    boost::optional<AddressPointer> getAddressPointerFromString(const DataConfiguration &config, const ScriptAccess &access, const std::string &addressString);
    
    std::vector<AddressPointer> getAddressPointersFromStrings(const DataConfiguration &config, const ScriptAccess &access, const std::vector<std::string> &addressStrings);
    
    std::vector<AddressPointer> getAddressPointersStartingWithPrefex(const DataConfiguration &config, const ScriptAccess &access, const std::string &prefix);
    size_t addressCount(const ScriptAccess &access);
    
    // Requires DataAccess
    #ifndef BLOCKSCI_WITHOUT_SINGLETON
    boost::optional<AddressPointer> getAddressPointerFromString(const std::string &addressString);
    std::vector<AddressPointer> getAddressPointersFromStrings(const std::vector<std::string> &addressStrings);
    std::vector<AddressPointer> getAddressPointersStartingWithPrefex(const std::string &prefix);
    size_t addressCount();
    #endif
}

std::ostream &operator<<(std::ostream &os, const blocksci::AddressPointer &address);

namespace std {
    template <>
    struct hash<blocksci::AddressPointer> {
        typedef blocksci::AddressPointer argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return (static_cast<size_t>(b.addressNum) << 32) + static_cast<size_t>(b.type);
        }
    };
}

#endif /* address_pointer_hpp */
