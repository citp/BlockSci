//
//  address_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address_pointer.hpp"
#include "script_access.hpp"
#include "script_first_seen_access.hpp"
#include "chain/transaction.hpp"
#include "scripts/script_info.hpp"
#include "scripts/address.hpp"
#include "scripts/address_types.hpp"
#include "raw_address_pointer.hpp"

#include <unordered_set>

namespace blocksci {
    
    AddressPointer::AddressPointer() : addressNum(0), type(ScriptType::Enum::NONSTANDARD) {}
    
    AddressPointer::AddressPointer(uint32_t addressNum_, ScriptType::Enum type_) : addressNum(addressNum_), type(type_) {}
    
    int AddressPointer::getDBType() const {
        return blocksci::getDBType(type);    }
    
    bool AddressPointer::isSpendable() const {
        return blocksci::isSpendable(type);
    }
    
    std::string AddressPointer::toString() const {
        if (addressNum == 0) {
            return "InvalidAddress()";
        } else {
            std::stringstream ss;
            ss << "AddressPointer(";
            ss << "addressNum=" << addressNum;
            ss << ", type=" << GetTxnOutputType(type);
            ss << ")";
            return ss.str();
        }
    }
    
    template<ScriptType::Enum type>
    struct FirstSeenFunctor {
        static uint32_t f(const ScriptFirstSeenAccess &access, uint32_t addressNum) {
            constexpr auto t = addressScriptType(type);
            return access.getFirstTxNum<t>(addressNum);
        }
    };
    
    uint32_t AddressPointer::getFirstTransactionIndex(const ScriptFirstSeenAccess &access) const {
        static constexpr auto table = make_dynamic_table<FirstSeenFunctor>();
        static constexpr std::size_t size = ScriptType::all.size();
        
        auto index = static_cast<size_t>(type);
        if (index >= size)
        {
            throw std::invalid_argument("combination of enum values is not valid");
        }
        return table[index](access, addressNum);
    }
    
    Transaction AddressPointer::getFirstTransaction(const ChainAccess &chain, const ScriptFirstSeenAccess &scriptsFirstSeen) const {
        return Transaction::txWithIndex(chain, getFirstTransactionIndex(scriptsFirstSeen));
    }

    std::unique_ptr<Address> AddressPointer::getAddress(const ScriptAccess &access) const {
        return Address::create(access, *this);
    }
    
    boost::optional<AddressPointer> getAddressPointerFromString(const DataConfiguration &config, const ScriptAccess &access, const std::string &addressString) {
        auto rawAddress = RawAddress::create(config, addressString);
        
        if (rawAddress) {
            if (rawAddress->type == ScriptType::Enum::PUBKEYHASH) {
                return access.findAddress<ScriptType::Enum::PUBKEYHASH>(rawAddress->hash);
            } else if (rawAddress->type == ScriptType::Enum::SCRIPTHASH) {
                return access.findAddress<ScriptType::Enum::SCRIPTHASH>(rawAddress->hash);
            }
        }
        return boost::none;
    }
    
    std::vector<AddressPointer> getAddressPointersFromStrings(const DataConfiguration &config, const ScriptAccess &access, const std::vector<std::string> &addressStrings) {
        
        std::unordered_set<RawAddress> rawAddresses;
        for (auto &addressString : addressStrings) {
            auto rawAddress = RawAddress::create(config, addressString);
            if (rawAddress) {
                rawAddresses.insert(*rawAddress);
            }
        }
        
        auto addresses = access.findMatchingAddresses<ScriptType::Enum::PUBKEYHASH>([&](const auto &nextAddress) {
            return rawAddresses.find(RawAddress(nextAddress)) != rawAddresses.end();
        });
        
        auto otherAddresses = access.findMatchingAddresses<ScriptType::Enum::SCRIPTHASH>([&](const auto &nextAddress) {
            return rawAddresses.find(RawAddress(nextAddress)) != rawAddresses.end();
        });
        
        addresses.insert(addresses.end(), otherAddresses.begin(), otherAddresses.end());
        
        return addresses;
    }
    
    template<ScriptType::Enum type>
    std::vector<AddressPointer> getAddressPointersStartingWithPrefex(const DataConfiguration &config, const ScriptAccess &access, const std::string &prefix) {
        return access.findMatchingAddresses<type>([&](const auto &nextAddress) {
            auto addressString = address::PubkeyHash(nextAddress.address).addressString(config);
            return addressString.compare(0, prefix.length(), prefix) == 0;
        });
    }
    
    std::vector<AddressPointer> getAddressPointersStartingWithPrefex(const DataConfiguration &config, const ScriptAccess &access, const std::string &prefix) {
        if (prefix.compare(0, 1, "1") == 0) {
            return getAddressPointersStartingWithPrefex<ScriptType::Enum::PUBKEYHASH>(config, access, prefix);
        } else if (prefix.compare(0, 1, "3") == 0) {
            return getAddressPointersStartingWithPrefex<ScriptType::Enum::SCRIPTHASH>(config, access, prefix);
        } else {
            return std::vector<AddressPointer>();
        }
    }
    
    size_t addressCount(const ScriptAccess &access) {
        return access.totalAddressCount();
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::AddressPointer &address) {
    os << address.toString();
    return os;
}

