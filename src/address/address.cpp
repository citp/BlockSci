//
//  address_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/12/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include <blocksci/address/address.hpp>
#include <blocksci/address/equiv_address.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/range_util.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <scripts/bitcoin_base58.hpp>
#include <scripts/bitcoin_segwit_addr.hpp>

#include <internal/dedup_address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/chain_access.hpp>
#include <internal/script_access.hpp>
#include <internal/address_index.hpp>
#include <internal/hash_index.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/unique.hpp>

#include <iostream>
#include <sstream>

namespace blocksci {
    
    ranges::any_view<OutputPointer> Address::getOutputPointers() const {
        return access->getAddressIndex().getOutputPointers(*this)
        | ranges::view::transform([](const InoutPointer &pointer) { return OutputPointer(pointer.txNum, pointer.inoutNum); });
    }
    
    ranges::any_view<Output> Address::getOutputs() const {
        return outputs(getOutputPointers(), *access);
    }
    
    ranges::any_view<Input> Address::getInputs() const {
        auto _access = access;
        return getOutputPointers()
        | ranges::view::transform([_access](const OutputPointer &pointer) { return Output(pointer, *_access).getSpendingInput(); })
        | flatMapOptionals();
    }
    
    std::vector<Transaction> Address::getTransactions() const {
        return blocksci::getTransactions(getOutputPointers(), *access);
    }

    ranges::any_view<Transaction> Address::getOutputTransactions() const {
        auto _access = access;
        auto uniqueTxNums = _access->getAddressIndex().getOutputPointers(*this)
        | ranges::view::transform([](const InoutPointer &pointer) -> uint32_t { return pointer.txNum; })
        | ranges::view::unique;
        return ranges::view::transform(uniqueTxNums, [_access](uint32_t txNum) { return Transaction(txNum, _access->getChain().getBlockHeight(txNum), *_access); });
    }
    
    std::vector<Transaction> Address::getInputTransactions() const {
        return blocksci::getInputTransactions(getOutputPointers(), *access);
    }
    
    bool Address::isSpendable() const {
        return blocksci::isSpendable(dedupType(type));
    }
    
    std::string Address::toString() const {
        if (scriptNum == 0) {
            return "InvalidAddress()";
        } else {
            return getScript().toString();
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

    AnyScript Address::getScript() const {
        return AnyScript{*this};
    }
    
    ScriptBase Address::getBaseScript() const {
        return ScriptBase(*this);
    }
    
    ranges::optional<Address> getAddressFromString(const std::string &addressString, DataAccess &access) {
        if (addressString.compare(0, access.config.chainConfig.segwitPrefix.size(), access.config.chainConfig.segwitPrefix) == 0) {
            std::pair<int, std::vector<uint8_t> > decoded = segwit_addr::decode(access.config.chainConfig.segwitPrefix, addressString);
            if (decoded.first == 0) {
                if (decoded.second.size() == 20) {
                    uint160 pubkeyHash(decoded.second.begin(), decoded.second.end());
                    ranges::optional<uint32_t> addressNum = access.getHashIndex().getPubkeyHashIndex(pubkeyHash);
                    if (addressNum) {
                        return Address{*addressNum, AddressType::WITNESS_PUBKEYHASH, access};
                    }
                } else if (decoded.second.size() == 32) {
                    uint256 scriptHash(decoded.second.begin(), decoded.second.end());
                    ranges::optional<uint32_t> addressNum = access.getHashIndex().getScriptHashIndex(scriptHash);
                    if (addressNum) {
                        return Address{*addressNum, AddressType::WITNESS_SCRIPTHASH, access};
                    }
                }
            }
            return ranges::nullopt;
        }
        CBitcoinAddress address{addressString};
        uint160 hash;
        blocksci::AddressType::Enum type;
        std::tie(hash, type) = address.Get(access.config.chainConfig);
        ranges::optional<uint32_t> addressNum = ranges::nullopt;
        if (type == AddressType::Enum::PUBKEYHASH) {
            addressNum = access.getHashIndex().getPubkeyHashIndex(hash);
        } else if (type == AddressType::Enum::SCRIPTHASH) {
            addressNum = access.getHashIndex().getScriptHashIndex(hash);
        }
        if (addressNum) {
            return Address{*addressNum, type, access};
        } else {
            return ranges::nullopt;
        }
    }
    
    template<AddressType::Enum type>
    std::vector<Address> getAddressesWithPrefixImp(const std::string &prefix, DataAccess &access) {
        std::vector<Address> addresses;
        auto count = access.getScripts().scriptCount(dedupType(type));
        for (uint32_t scriptNum = 1; scriptNum <= count; scriptNum++) {
            ScriptAddress<type> script(scriptNum, access);
            if (script.addressString().compare(0, prefix.length(), prefix) == 0) {
                addresses.emplace_back(scriptNum, type, access);
            }
        }
        return addresses;
    }
    
    std::vector<Address> getAddressesWithPrefix(const std::string &prefix, DataAccess &access) {
        if (prefix.compare(0, 1, "1") == 0) {
            return getAddressesWithPrefixImp<AddressType::Enum::PUBKEYHASH>(prefix, access);
        } else if (prefix.compare(0, 1, "3") == 0) {
            return getAddressesWithPrefixImp<AddressType::Enum::SCRIPTHASH>(prefix, access);
        } else {
            return {};
        }
    }
    
    std::string fullTypeImp(const Address &address, DataAccess &access) {
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
            case DedupAddressType::PUBKEY: {
                break;
            }
            case DedupAddressType::NONSTANDARD: {
                break;
            }
            case DedupAddressType::WITNESS_UNKNOWN: {
                break;
            }
            case DedupAddressType::NULL_DATA: {
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

    /* Get the balance of the address by:
     * 1) getting all outputs that are linked to the address, and
     * 2) adding up all unspent outputs.
     */
    int64_t Address::calculateBalance(BlockHeight height) const {
        return balance(height, outputs(getOutputPointers(), *access));
    }
}

