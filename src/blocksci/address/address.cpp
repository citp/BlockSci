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

namespace blocksci {
    
    Address::Address() : scriptNum(0), type(AddressType::Enum::NONSTANDARD) {}
    
    Address::Address(uint32_t addressNum_, AddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
    
    
    DedupAddress Address::dedup() const {
        return DedupAddress{scriptNum, scriptType(type)};
    }
    
    
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
            ss << ", type=" << addressName(type);
            ss << ")";
            return ss.str();
        }
    }
    
    void visit(const Address &address, const std::function<bool(const Address &)> &visitFunc, const ScriptAccess &scripts) {
        if (visitFunc(address)) {
            std::function<void(const blocksci::Address &)> nestedVisitor = [&](const blocksci::Address &nestedAddress) {
                visit(nestedAddress, visitFunc, scripts);
            };
            auto script = address.getScript(scripts);
            script.visitPointers(nestedVisitor);
        }
    }

    AnyScript Address::getScript(const ScriptAccess &access) const {
        return AnyScript{*this, access};
    }
    
    size_t addressCount(const ScriptAccess &access) {
        return access.totalAddressCount();
    }
    
    uint64_t Address::calculateBalance(BlockHeight height, const AddressIndex &index, const ChainAccess &chain) const {
        uint64_t value = 0;
        if (height == 0) {
            for (auto &output : index.getOutputs(*this, chain)) {
                if (!output.isSpent()) {
                    value += output.getValue();
                }
            }
        } else {
            for (auto &output : index.getOutputs(*this, chain)) {
                if (output.blockHeight <= height && (!output.isSpent() || output.getSpendingTx()->blockHeight > height)) {
                    value += output.getValue();
                }
            }
        }
        return value;
    }
    
    std::vector<Output> Address::getOutputs(const AddressIndex &index, const ChainAccess &chain) const {
        return index.getOutputs(*this, chain);
    }
    
    std::vector<Input> Address::getInputs(const AddressIndex &index, const ChainAccess &chain) const {
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
    
    ranges::optional<Address> getAddressFromString(const DataConfiguration &config, HashIndex &index, const std::string &addressString) {
        if (std::equal(config.segwitPrefix.begin(), config.segwitPrefix.end(), addressString.begin(), addressString.end() + config.segwitPrefix.size())) {
            std::pair<int, std::vector<uint8_t> > decoded = segwit_addr::decode(config.segwitPrefix, addressString);
            if (decoded.first == 1) {
                if (decoded.second.size() == 20) {
                    uint160 pubkeyHash(decoded.second.begin(), decoded.second.end());
                    uint32_t addressNum = index.getPubkeyHashIndex(pubkeyHash);
                    if (addressNum > 0) {
                        return Address{addressNum, AddressType::WITNESS_PUBKEYHASH};
                    }
                } else if (decoded.second.size() == 32) {
                    uint256 scriptHash(decoded.second.begin(), decoded.second.end());
                    uint32_t addressNum = index.getScriptHashIndex(scriptHash);
                    if (addressNum > 0) {
                        return Address{addressNum, AddressType::WITNESS_SCRIPTHASH};
                    }
                }
            }
            return ranges::nullopt;
        }
        CBitcoinAddress address{addressString};
        uint160 hash;
        blocksci::AddressType::Enum type;
        std::tie(hash, type) = address.Get(config);
        if (type == AddressType::Enum::NONSTANDARD) {
            return ranges::nullopt;
        }
        uint32_t addressNum = 0;
        if (type == AddressType::Enum::PUBKEYHASH) {
            addressNum = index.getPubkeyHashIndex(hash);
        } else if (type == AddressType::Enum::SCRIPTHASH) {
            addressNum = index.getScriptHashIndex(hash);
        }
        if (addressNum > 0) {
            return Address{addressNum, type};
        }
        return ranges::nullopt;
    }
    
    template<AddressType::Enum type>
    std::vector<Address> getAddressesWithPrefixImp(const std::string &prefix, const ScriptAccess &scripts) {
        std::vector<Address> addresses;
        auto count = scripts.scriptCount(scriptType(type));
        for (uint32_t scriptNum = 1; scriptNum <= count; scriptNum++) {
            ScriptAddress<type> script(scripts, scriptNum);
            if (script.addressString().compare(0, prefix.length(), prefix) == 0) {
                addresses.push_back(Address(scriptNum, type));
            }
        }
        return addresses;
    }
    
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix, const ScriptAccess &scripts) {
        if (prefix.compare(0, 1, "1") == 0) {
            return getAddressesWithPrefixImp<AddressType::Enum::PUBKEYHASH>(prefix, scripts);
        } else if (prefix.compare(0, 1, "3") == 0) {
            return getAddressesWithPrefixImp<AddressType::Enum::SCRIPTHASH>(prefix, scripts);
        } else {
            return {};
        }
    }
    
    std::string fullTypeImp(const Address &address, const ScriptAccess &scripts) {
        std::stringstream ss;
        ss << addressName(address.type);
        switch (scriptType(address.type)) {
            case ScriptType::Enum::SCRIPTHASH: {
                auto script = script::ScriptHash(scripts, address.scriptNum);
                auto wrapped = script.getWrappedAddress();
                if (wrapped) {
                    ss << "/" << fullTypeImp(*wrapped, scripts);
                }
                break;
            }
            case ScriptType::Enum::MULTISIG: {
                auto script = script::Multisig(scripts, address.scriptNum);
                ss << int(script.required) << "Of" << int(script.total);
                break;
            }
            default: {
                break;
            }
        }
        return ss.str();
    }

    std::string Address::fullType(const ScriptAccess &script) const {
        return fullTypeImp(*this, script);
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::Address &address) {
    os << address.toString();
    return os;
}

