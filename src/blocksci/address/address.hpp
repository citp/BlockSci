//
//  address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#ifndef address_hpp
#define address_hpp

#include "address_fwd.hpp"
#include "address_types.hpp"

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <range/v3/utility/optional.hpp>

#include <functional>
#include <vector>

namespace blocksci {
    class DataAccess;
    struct DataConfiguration;
    class EquivAddress;
    
    struct RawAddress {
        uint32_t scriptNum;
        AddressType::Enum type;
        
        RawAddress() {}
        RawAddress(uint32_t addressNum_, AddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
        explicit RawAddress(const Address &address);
    };
    
    class Address {
        const DataAccess *access;
        
    public:
        uint32_t scriptNum;
        AddressType::Enum type;
        
        Address();
        Address(uint32_t addressNum, AddressType::Enum type, const DataAccess &access);
        Address(const RawAddress &raw, const DataAccess &access_) : access(&access_), scriptNum(raw.scriptNum), type(raw.type) {}
        
        const DataAccess &getAccess() const {
            return *access;
        }
        
        bool isSpendable() const;
        
        bool operator==(const Address& other) const {
            return type == other.type && scriptNum == other.scriptNum;
        }
        
        bool operator!=(const Address& other) const {
            return !operator==(other);
        }
        
        std::string toString() const;
        
        AnyScript getScript() const;
        
        uint64_t calculateBalance(BlockHeight height) const;
        
        EquivAddress getEquivAddresses(bool nestedEquivalent) const;
        
        std::vector<OutputPointer> getOutputPointers() const;
        
        std::vector<Output> getOutputs() const;
        std::vector<Input> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        std::vector<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
        
        std::string fullType() const;
    };
    
    void visit(const Address &address, const std::function<bool(const Address &)> &visitFunc);
    void visit(const RawAddress &address, const std::function<bool(const RawAddress &)> &visitFunc, const ScriptAccess &scripts);
    
    ranges::optional<Address> getAddressFromString(const std::string &addressString, const DataAccess &access);
    
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix, const DataAccess &access);

    inline RawAddress::RawAddress(const Address &address) : scriptNum(address.scriptNum), type(address.type) {}
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
