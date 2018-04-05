//
//  address_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "address.hpp"
#include "dedup_address.hpp"
#include "equiv_address.hpp"
#include "address_info.hpp"
#include <blocksci/scripts/bitcoin_base58.hpp>
#include <blocksci/scripts/bitcoin_segwit_addr.hpp>
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

#include <unordered_set>
#include <iostream>

namespace blocksci {
    
    Address::Address() : access(nullptr), scriptNum(0), type(AddressType::Enum::NONSTANDARD) {}
    
    Address::Address(uint32_t addressNum_, AddressType::Enum type_, const DataAccess &access_) : access(&access_), scriptNum(addressNum_), type(type_) {}
    
    bool Address::isSpendable() const {
        return blocksci::isSpendable(dedupType(type));
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
            std::function<void(const Address &)> nestedVisitor = [&](const Address &nestedAddress) {
                visit(nestedAddress, visitFunc);
            };
            auto script = address.getScript();
            script.visitPointers(nestedVisitor);
        }
    }
    
    void visit(const RawAddress &address, const std::function<bool(const RawAddress &)> &visitFunc, const ScriptAccess &scripts) {
        if (visitFunc(address)) {
            std::function<void(const RawAddress &)> nestedVisitor = [&](const RawAddress &nestedAddress) {
                visit(nestedAddress, visitFunc, scripts);
            };
            AnyScriptData script{address, scripts};
            script.visitPointers(nestedVisitor);
        }
    }

    AnyScript Address::getScript() const {
        return AnyScript{*this, *access};
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
        auto count = access.scripts->scriptCount(dedupType(type));
        for (uint32_t scriptNum = 1; scriptNum <= count; scriptNum++) {
            ScriptAddress<type> script(scriptNum, access);
            if (script.addressString().compare(0, prefix.length(), prefix) == 0) {
                addresses.emplace_back(scriptNum, type, access);
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
        switch (dedupType(address.type)) {
            case DedupAddressType::SCRIPTHASH: {
                auto script = script::ScriptHash(address.scriptNum, access);
                auto wrapped = script.getWrappedAddress();
                if (wrapped) {
                    ss << "/" << fullTypeImp(*wrapped, access);
                }
                break;
            }
            case DedupAddressType::MULTISIG: {
                auto script = script::Multisig(address.scriptNum, access);
                ss << int(script.getRequired()) << "Of" << int(script.getTotal());
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
    
    EquivAddress Address::getEquivAddresses(bool nestedEquivalent) const {
        return EquivAddress{*this, nestedEquivalent};
    }
    
    uint64_t Address::calculateBalance(BlockHeight height) const {
        return blocksci::calculateBalance(getOutputPointers(), height, *access);
    }
    
    std::vector<OutputPointer> Address::getOutputPointers() const {
        return access->addressIndex->getOutputPointers(*this);
    }
    
    std::vector<Output> Address::getOutputs() const {
        return blocksci::getOutputs(getOutputPointers(), *access);
    }
    
    std::vector<Input> Address::getInputs() const {
        return blocksci::getInputs(getOutputPointers(), *access);
    }
    
    std::vector<Transaction> Address::getTransactions() const {
        return blocksci::getTransactions(getOutputPointers(), *access);
    }
    
    std::vector<Transaction> Address::getOutputTransactions() const {
        return blocksci::getOutputTransactions(getOutputPointers(), *access);
    }
    
    std::vector<Transaction> Address::getInputTransactions() const {
        return blocksci::getInputTransactions(getOutputPointers(), *access);
    }
}

