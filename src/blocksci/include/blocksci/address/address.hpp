//
//  address.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#ifndef address_hpp
#define address_hpp

#include <blocksci/blocksci_export.h>

#include "address_fwd.hpp"

#include <blocksci/core/address_types.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/util/data_access.hpp>

#include <range/v3/utility/optional.hpp>
#include <range/v3/view/any_view.hpp>

#include <functional>
#include <vector>

namespace blocksci {
    class DataAccess;
    struct DataConfiguration;
    class EquivAddress;
    
    class BLOCKSCI_EXPORT Address {
        DataAccess *access;
        
    public:
        uint32_t scriptNum;
        AddressType::Enum type;
        
        Address() : access(nullptr), scriptNum(0), type(AddressType::Enum::NONSTANDARD) {}
        Address(uint32_t addressNum_, AddressType::Enum type_, DataAccess &access_) : access(&access_), scriptNum(addressNum_), type(type_) {}
        
        DataAccess &getAccess() const {
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
        
        uint32_t getScriptNum() const {
            return scriptNum;
        }

        AddressType::Enum getType() const {
            return type;
        }

        AnyScript getScript() const;
        
        EquivAddress getEquivAddresses(bool nestedEquivalent) const;
        
        auto getOutputPointers() const {
            return access->addressIndex.getOutputPointers(*this);
        }
        
        int64_t calculateBalance(BlockHeight height);
        ranges::any_view<Output> getOutputs();
        std::vector<Input> getInputs();
        std::vector<Transaction> getTransactions();
        std::vector<Transaction> getOutputTransactions();
        std::vector<Transaction> getInputTransactions();
        
        std::string fullType() const;
    };
    
    void BLOCKSCI_EXPORT visit(const Address &address, const std::function<bool(const Address &)> &visitFunc);
    
    ranges::optional<Address> BLOCKSCI_EXPORT getAddressFromString(const std::string &addressString, DataAccess &access);
    
    std::vector<Address> BLOCKSCI_EXPORT getAddressesWithPrefix(const std::string &prefix, DataAccess &access);
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
