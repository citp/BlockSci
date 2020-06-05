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
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/core/address_types.hpp>
#include <blocksci/core/raw_address.hpp>
#include <blocksci/core/typedefs.hpp>
#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/utility/optional.hpp>

#include <functional>
#include <unordered_set>
#include <vector>

/** Namespace of the analysis library */
namespace blocksci {
    class DataAccess;

    /** Represents an address of the blockchain
     *
     * In BlockSci, an address consists of a script number (Address.scriptNum) and the address type.
     * Script data is stored in separate files for every address type in `scripts/` and can be accessed by ScriptAccess.
     * The scriptNum represents an index to the scripts file for this address type.
     */
    class BLOCKSCI_EXPORT Address {
        DataAccess *access;
        
    public:
        /** Script number of the address, represents an index in the scripts file for this address type */
        uint32_t scriptNum;

        /** Address type of this address */
        AddressType::Enum type;
        
        Address() : access(nullptr), scriptNum(0), type(AddressType::Enum::NONSTANDARD) {}
        Address(uint32_t addressNum_, AddressType::Enum type_, DataAccess &access_) : access(&access_), scriptNum(addressNum_), type(type_) {}
        Address(const RawAddress &raw, DataAccess &access_) : Address(raw.scriptNum, raw.type, access_) {}
        
        operator RawAddress() const {
            return RawAddress{scriptNum, type};
        }
        
        DataAccess &getAccess() const {
            return *access;
        }
        
        bool isSpendable() const;
        
        bool operator==(const Address& other) const {
            return std::tie(type, scriptNum) == std::tie(other.type, other.scriptNum);
        }
        
        bool operator!=(const Address& other) const {
            return std::tie(type, scriptNum) != std::tie(other.type, other.scriptNum);
        }
        
        bool operator<(const Address& other) const {
            return std::tie(type, scriptNum) < std::tie(other.type, other.scriptNum);
        }
        
        bool operator<=(const Address& other) const {
            return std::tie(type, scriptNum) <= std::tie(other.type, other.scriptNum);
        }
        
        bool operator>(const Address& other) const {
            return std::tie(type, scriptNum) > std::tie(other.type, other.scriptNum);
        }
        
        bool operator>=(const Address& other) const {
            return std::tie(type, scriptNum) >= std::tie(other.type, other.scriptNum);
        }
        
        std::string toString() const;
        
        uint32_t getScriptNum() const {
            return scriptNum;
        }

        AddressType::Enum getType() const {
            return type;
        }

        AnyScript getScript() const;
        
        ScriptBase getBaseScript() const;
        
        EquivAddress getEquivAddresses(bool nestedEquivalent) const;
        
        ranges::any_view<OutputPointer> getOutputPointers() const;
        int64_t calculateBalance(BlockHeight height) const;
        ranges::any_view<Output> getOutputs() const;
        ranges::any_view<Input> getInputs() const;
        ranges::any_view<Transaction> getTransactions() const;
        ranges::any_view<Transaction> getOutputTransactions() const;
        ranges::any_view<Transaction> getInputTransactions() const;
        
        std::string fullType() const;
    };
    
    void BLOCKSCI_EXPORT visit(const Address &address, const std::function<bool(const Address &)> &visitFunc);
    
    ranges::optional<Address> BLOCKSCI_EXPORT getAddressFromString(const std::string &addressString, DataAccess &access);
    
    std::vector<Address> BLOCKSCI_EXPORT getAddressesWithPrefix(const std::string &prefix, DataAccess &access);
    
    inline size_t hashAddress(uint32_t scriptNum, AddressType::Enum type) {
        return (static_cast<size_t>(scriptNum) << 32) + static_cast<size_t>(type);
    }
}



namespace std {
    template <>
    struct hash<blocksci::Address> {
        typedef blocksci::Address argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return blocksci::hashAddress(b.scriptNum, b.type);
        }
    };
}

#endif /* address_hpp */
