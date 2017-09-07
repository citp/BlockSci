//
//  address_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address.hpp"
#include "address_index.hpp"
#include "address_info.hpp"
#include "raw_address_pointer.hpp"
#include "scripts/script_first_seen_access.hpp"
#include "scripts/script_access.hpp"
#include "scripts/script.hpp"
#include "scripts/scripthash_script.hpp"
#include "scripts/scriptsfwd.hpp"
#include "chain/transaction.hpp"

#include <unordered_set>

namespace blocksci {
    
    Address::Address() : addressNum(0), type(AddressType::Enum::NONSTANDARD) {}
    
    Address::Address(uint32_t addressNum_, AddressType::Enum type_) : addressNum(addressNum_), type(type_) {}
    
    int Address::getDBType() const {
        return blocksci::getDBType(type);    }
    
    bool Address::isSpendable() const {
        return blocksci::isSpendable(type);
    }
    
    std::string Address::toString() const {
        if (addressNum == 0) {
            return "InvalidAddress()";
        } else {
            std::stringstream ss;
            ss << "Address(";
            ss << "addressNum=" << addressNum;
            ss << ", type=" << GetTxnOutputType(type);
            ss << ")";
            return ss.str();
        }
    }
    
    template<AddressType::Enum type>
    struct FirstSeenFunctor {
        static uint32_t f(const ScriptFirstSeenAccess &access, uint32_t addressNum) {
            constexpr auto t = scriptType(type);
            return access.getFirstTxNum<t>(addressNum);
        }
    };
    
    uint32_t Address::getFirstTransactionIndex(const ScriptFirstSeenAccess &access) const {
        static constexpr auto table = make_dynamic_table<AddressType, FirstSeenFunctor>();
        static constexpr std::size_t size = AddressType::all.size();
        
        auto index = static_cast<size_t>(type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](access, addressNum);
    }
    
    Transaction Address::getFirstTransaction(const ChainAccess &chain, const ScriptFirstSeenAccess &scriptsFirstSeen) const {
        return Transaction::txWithIndex(chain, getFirstTransactionIndex(scriptsFirstSeen));
    }

    std::unique_ptr<Script> Address::getScript(const ScriptAccess &access) const {
        return Script::create(access, *this);
    }
    
    size_t addressCount(const ScriptAccess &access) {
        return access.totalAddressCount();
    }
    
    std::vector<const Output *> Address::getOutputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputs(*this, chain);
    }
    
    std::vector<const Input *> Address::getInputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getInputs(*this, chain);
    }
    
    std::vector<Transaction> Address::getTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getTransactions(*this, chain);
    }
    
    std::vector<Transaction> Address::getOutputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputTransactions(*this, chain);
    }
    
    std::vector<Transaction> Address::getInputTransactions(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getInputTransactions(*this, chain);
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::Address &address) {
    os << address.toString();
    return os;
}

