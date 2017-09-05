//
//  address_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address.hpp"
#include "address_first_seen_access.hpp"
#include "address_index.hpp"
#include "address_info.hpp"
#include "raw_address_pointer.hpp"
#include "scripts/script_access.hpp"
#include "scripts/script.hpp"
#include "scripts/pubkeyhash_script.hpp"
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
        static uint32_t f(const AddressFirstSeenAccess &access, uint32_t addressNum) {
            constexpr auto t = addressAddressType(type);
            return access.getFirstTxNum<t>(addressNum);
        }
    };
    
    uint32_t Address::getFirstTransactionIndex(const AddressFirstSeenAccess &access) const {
        static constexpr auto table = make_dynamic_table<FirstSeenFunctor>();
        static constexpr std::size_t size = AddressType::all.size();
        
        auto index = static_cast<size_t>(type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](access, addressNum);
    }
    
    Transaction Address::getFirstTransaction(const ChainAccess &chain, const AddressFirstSeenAccess &scriptsFirstSeen) const {
        return Transaction::txWithIndex(chain, getFirstTransactionIndex(scriptsFirstSeen));
    }

    std::unique_ptr<Script> Address::getScript(const ScriptAccess &access) const {
        return Script::create(access, *this);
    }
    
    boost::optional<Address> getAddressFromString(const DataConfiguration &config, const ScriptAccess &access, const std::string &addressString) {
        auto rawAddress = RawAddress::create(config, addressString);
        
        if (rawAddress) {
            if (rawAddress->type == AddressType::Enum::PUBKEYHASH) {
                return access.findAddress<AddressType::Enum::PUBKEYHASH>(rawAddress->hash);
            } else if (rawAddress->type == AddressType::Enum::SCRIPTHASH) {
                return access.findAddress<AddressType::Enum::SCRIPTHASH>(rawAddress->hash);
            }
        }
        return boost::none;
    }
    
    std::vector<Address> getAddressesFromStrings(const DataConfiguration &config, const ScriptAccess &access, const std::vector<std::string> &addressStrings) {
        
        std::unordered_set<RawAddress> rawAddresses;
        for (auto &addressString : addressStrings) {
            auto rawAddress = RawAddress::create(config, addressString);
            if (rawAddress) {
                rawAddresses.insert(*rawAddress);
            }
        }
        
        auto addresses = access.findMatchingAddresses<AddressType::Enum::PUBKEYHASH>([&](const auto &nextAddress) {
            return rawAddresses.find(RawAddress(nextAddress)) != rawAddresses.end();
        });
        
        auto otherAddresses = access.findMatchingAddresses<AddressType::Enum::SCRIPTHASH>([&](const auto &nextAddress) {
            return rawAddresses.find(RawAddress(nextAddress)) != rawAddresses.end();
        });
        
        addresses.insert(addresses.end(), otherAddresses.begin(), otherAddresses.end());
        
        return addresses;
    }
    
    template<AddressType::Enum type>
    std::vector<Address> getAddressesWithPrefix(const DataConfiguration &config, const ScriptAccess &access, const std::string &prefix) {
        return access.findMatchingAddresses<type>([&](const auto &nextAddress) {
            auto addressString = ScriptAddress<type>(nextAddress.address).addressString(config);
            return addressString.compare(0, prefix.length(), prefix) == 0;
        });
    }
    
    std::vector<Address> getAddressesWithPrefix(const DataConfiguration &config, const ScriptAccess &access, const std::string &prefix) {
        if (prefix.compare(0, 1, "1") == 0) {
            return getAddressesWithPrefix<AddressType::Enum::PUBKEYHASH>(config, access, prefix);
        } else if (prefix.compare(0, 1, "3") == 0) {
            return getAddressesWithPrefix<AddressType::Enum::SCRIPTHASH>(config, access, prefix);
        } else {
            return std::vector<Address>();
        }
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

