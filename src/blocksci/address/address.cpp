//
//  address_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address.hpp"
#include "equiv_address.hpp"
#include "address_info.hpp"
#include "scripts/bitcoin_base58.hpp"
#include "scripts/bitcoin_segwit_addr.hpp"
#include "scripts/script_access.hpp"
#include "scripts/scripts_fwd.hpp"
#include "scripts/script_variant.hpp"
#include "chain/transaction.hpp"
#include "chain/output.hpp"
#include "index/address_index.hpp"
#include "index/hash_index.hpp"

#include <unordered_set>
#include <iostream>

namespace blocksci {
    
    Address::Address() : access(nullptr), scriptNum(0), type(AddressType::Enum::NONSTANDARD) {}
    
    Address::Address(uint32_t addressNum_, AddressType::Enum type_, const DataAccess &access_) : access(&access_), scriptNum(addressNum_), type(type_) {}
    
    
    EquivAddress Address::equiv() const {
        return EquivAddress{scriptNum, equivType(type)};
    }
    
    
    bool Address::isSpendable() const {
        return blocksci::isSpendable(equivType(type));
    }
    
    std::string Address::toString() const {
        if (scriptNum == 0) {
            return "InvalidAddress()";
        } else {
            std::stringstream ss;
            ss << "Address(";
            ss << "addressNum=" << scriptNum;
            ss << ", type=" << addressName(type);
            ss << ")";
            return ss.str();
        }
    }
    
    void visit(const Address &address, const std::function<bool(const Address &)> &visitFunc) {
        if (visitFunc(address)) {
            std::function<void(const blocksci::Address &)> nestedVisitor = [&](const blocksci::Address &nestedAddress) {
                visit(nestedAddress, visitFunc);
            };
            auto script = address.getScript();
            script.visitPointers(nestedVisitor);
        }
    }

    AnyScript Address::getScript() const {
        return AnyScript{*this, *access};
    }
    
    size_t addressCount(const ScriptAccess &access) {
        return access.totalAddressCount();
    }
    
    uint64_t Address::calculateBalance(BlockHeight height) const {
        uint64_t value = 0;
        if (height == 0) {
            for (auto &output : access->getOutputs(*this)) {
                if (!output.isSpent()) {
                    value += output.getValue();
                }
            }
        } else {
            for (auto &output : access->getOutputs(*this)) {
                if (output.blockHeight <= height && (!output.isSpent() || output.getSpendingTx()->blockHeight > height)) {
                    value += output.getValue();
                }
            }
        }
        return value;
    }
    
    std::vector<Output> Address::getOutputs() const {
        return access->getOutputs(*this);
    }
    
    std::vector<Input> Address::getInputs() const {
        return access->getInputs(*this);
    }
    
    std::vector<Transaction> Address::getTransactions() const {
        return access->getTransactions(*this);
    }
    
    std::vector<Transaction> Address::getOutputTransactions() const {
        return access->getOutputTransactions(*this);
    }
    
    std::vector<Transaction> Address::getInputTransactions() const {
        return access->getInputTransactions(*this);
    }
    
    ranges::optional<Address> getAddressFromString(const std::string &addressString, const DataAccess &access) {
        if (addressString.compare(0, access.config.segwitPrefix.size(), access.config.segwitPrefix) == 0) {
            std::pair<int, std::vector<uint8_t> > decoded = segwit_addr::decode(access.config.segwitPrefix, addressString);
            if (decoded.first == 0) {
                if (decoded.second.size() == 20) {
                    uint160 pubkeyHash(decoded.second.begin(), decoded.second.end());
                    uint32_t addressNum = access.hashIndex->getPubkeyHashIndex(pubkeyHash);
                    if (addressNum > 0) {
                        return Address{addressNum, AddressType::WITNESS_PUBKEYHASH, access};
                    }
                } else if (decoded.second.size() == 32) {
                    uint256 scriptHash(decoded.second.begin(), decoded.second.end());
                    uint32_t addressNum = access.hashIndex->getScriptHashIndex(scriptHash);
                    if (addressNum > 0) {
                        return Address{addressNum, AddressType::WITNESS_SCRIPTHASH, access};
                    }
                }
            }
            return ranges::nullopt;
        }
        CBitcoinAddress address{addressString};
        uint160 hash;
        blocksci::AddressType::Enum type;
        std::tie(hash, type) = address.Get(access.config);
        if (type == AddressType::Enum::NONSTANDARD) {
            return ranges::nullopt;
        }
        uint32_t addressNum = 0;
        if (type == AddressType::Enum::PUBKEYHASH) {
            addressNum = access.hashIndex->getPubkeyHashIndex(hash);
        } else if (type == AddressType::Enum::SCRIPTHASH) {
            addressNum = access.hashIndex->getScriptHashIndex(hash);
        }
        if (addressNum > 0) {
            return Address{addressNum, type, access};
        }
        return ranges::nullopt;
    }
    
    template<AddressType::Enum type>
    std::vector<Address> getAddressesWithPrefixImp(const std::string &prefix, const DataAccess &access) {
        std::vector<Address> addresses;
        auto count = access.scripts->scriptCount(equivType(type));
        for (uint32_t scriptNum = 1; scriptNum <= count; scriptNum++) {
            ScriptAddress<type> script(scriptNum, access);
            if (script.addressString().compare(0, prefix.length(), prefix) == 0) {
                addresses.push_back(Address(scriptNum, type, access));
            }
        }
        return addresses;
    }
    
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix, const DataAccess &access) {
        if (prefix.compare(0, 1, "1") == 0) {
            return getAddressesWithPrefixImp<AddressType::Enum::PUBKEYHASH>(prefix, access);
        } else if (prefix.compare(0, 1, "3") == 0) {
            return getAddressesWithPrefixImp<AddressType::Enum::SCRIPTHASH>(prefix, access);
        } else {
            return {};
        }
    }
    
    std::string fullTypeImp(const Address &address, const DataAccess &access) {
        std::stringstream ss;
        ss << addressName(address.type);
        switch (equivType(address.type)) {
            case EquivAddressType::SCRIPTHASH: {
                auto script = script::ScriptHash(address.scriptNum, access);
                auto wrapped = script.getWrappedAddress();
                if (wrapped) {
                    ss << "/" << fullTypeImp(*wrapped, access);
                }
                break;
            }
            case EquivAddressType::MULTISIG: {
                auto script = script::Multisig(address.scriptNum, access);
                ss << int(script.required) << "Of" << int(script.total);
                break;
            }
            default: {
                break;
            }
        }
        return ss.str();
    }

    std::string Address::fullType() const {
        return fullTypeImp(*this, *access);
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::Address &address) {
    os << address.toString();
    return os;
}

