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
#include "scripts/scripts.hpp"
#include "scripts/scriptsfwd.hpp"
#include "scripts/script_variant.hpp"
#include "chain/transaction.hpp"
#include "hash_index.hpp"

#include <boost/variant.hpp>

#include <unordered_set>

namespace blocksci {
    
    Address::Address() : scriptNum(0), type(AddressType::Enum::NONSTANDARD) {}
    
    Address::Address(uint32_t addressNum_, AddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
    
    bool Address::isSpendable() const {
        return blocksci::isSpendable(scriptType(type));
    }
    
    std::string Address::toString() const {
        if (scriptNum == 0) {
            return "InvalidAddress()";
        } else {
            std::stringstream ss;
            ss << "Address(";
            ss << "addressNum=" << scriptNum;
            ss << ", type=" << GetTxnOutputType(type);
            ss << ")";
            return ss.str();
        }
    }
    
    bool Address::operator!=(const Script &other) const {
        return scriptNum == other.scriptNum && scriptType(type) == other.type;
    }
    
    void visit(const Address &address, const std::function<bool(const Address &)> &visitFunc, const ScriptAccess &scripts) {
        if (visitFunc(address)) {
            std::function<void(const blocksci::Address &)> nestedVisitor = [&](const blocksci::Address &nestedAddress) {
                visit(nestedAddress, visitFunc, scripts);
            };
            address.getScript(scripts).visitPointers(nestedVisitor);
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
        return table[index](access, scriptNum);
    }
    
    Transaction Address::getFirstTransaction(const ChainAccess &chain, const ScriptFirstSeenAccess &scriptsFirstSeen) const {
        return Transaction::txWithIndex(chain, getFirstTransactionIndex(scriptsFirstSeen));
    }

    AnyScript Address::getScript(const ScriptAccess &access) const {
        return AnyScript{*this, access};
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
    
    boost::optional<Address> getAddressFromString(const DataConfiguration &config, const HashIndex &index, const std::string &addressString) {
        auto rawAddress = RawAddress::create(config, addressString);
        
        if (rawAddress) {
            uint32_t addressNum = 0;
            if (rawAddress->type == AddressType::Enum::PUBKEYHASH) {
                addressNum = index.getPubkeyHashIndex(rawAddress->hash);
            } else if (rawAddress->type == AddressType::Enum::SCRIPTHASH) {
                addressNum = index.getScriptHashIndex(rawAddress->hash);
            }
            if (addressNum > 0) {
                return Address{addressNum, rawAddress->type};
            }
        }
        return boost::none;
    }
    
}

std::ostream &operator<<(std::ostream &os, const blocksci::Address &address) {
    os << address.toString();
    return os;
}

