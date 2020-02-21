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
#include <range/v3/algorithm/min.hpp>

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

    ranges::any_view<Transaction> Address::getOutputTransactions() const {
        auto _access = access;
        return _access->getAddressIndex().getOutputPointers(*this)
        | ranges::view::transform([](const InoutPointer &pointer) -> uint32_t { return pointer.txNum; })
        | ranges::view::unique
        | ranges::view::transform([_access](uint32_t txNum) { return Transaction(txNum, _access->getChain().getBlockHeight(txNum), *_access); });
    }
    
    ranges::any_view<Transaction> Address::getInputTransactions() const {
        auto _access = access;
        Address searchAddress = *this;
        return getOutputPointers()
        | ranges::view::transform([_access, searchAddress](auto pointer) -> ranges::optional<Transaction> {
            auto spendingTx = Output(std::forward<decltype(pointer)>(pointer), *_access).getSpendingTx();
            if (spendingTx) {
                RANGES_FOR(auto input, spendingTx->inputs()) {
                    if (input.getAddress() == searchAddress) {
                        if (input.getSpentOutputPointer() == pointer) {
                            return input.transaction();
                        } else {
                            return ranges::nullopt;
                        }
                    }
                }
                assert(false);
            } else {
                return ranges::nullopt;
            }
        })
        | flatMapOptionals();
    }
    
    class AddressAllTxRange : public ranges::view_facade<AddressAllTxRange> {
        friend ranges::range_access;
        using PointerView = ranges::any_view<OutputPointer>;
        
        DataAccess *access;
        Address searchAddress;
        PointerView pointers;
        
        struct cursor {
        private:
            DataAccess *access;
            Address searchAddress;
            ranges::iterator_t<PointerView> it;
            ranges::sentinel_t<PointerView> end;
            ranges::optional<Transaction> outTx;
            ranges::optional<Transaction> inTx;
            
            
            bool initializeFromCurrentOutput() {
                auto pointer = *it;
                Output out(pointer, *access);
                auto tx = out.transaction();
                RANGES_FOR(auto output, tx.outputs()) {
                    if (output.getAddress() == searchAddress) {
                        if (output.pointer == pointer) {
                            outTx = tx;
                        } else {
                            outTx = ranges::nullopt;
                        }
                        break;
                    }
                }
                
                inTx = out.getSpendingTx();
                if (inTx) {
                    RANGES_FOR(auto output, inTx->outputs()) {
                        if (output.getAddress() == searchAddress) {
                            inTx = ranges::nullopt;
                            break;
                        }
                    }
                }
                if (inTx) {
                    RANGES_FOR(auto input, inTx->inputs()) {
                        if (input.getAddress() == searchAddress) {
                            if (input.getSpentOutputPointer() != pointer) {
                                inTx = ranges::nullopt;
                            }
                            break;
                        }
                    }
                }
                
                return inTx.has_value() || outTx.has_value();
            }
            
            bool getNext() {
                ++it;
                if (it == end) {
                    return true;
                }
                return initializeFromCurrentOutput();
            }
            
            
            void satisfyNext() {
                while(!getNext()) {}
            }
        public:
            cursor() = default;
            
            cursor(ranges::iterator_t<PointerView> && it_, ranges::sentinel_t<PointerView> && end_,  const Address &searchAddress_, DataAccess *access_) :
            access(access_), searchAddress(searchAddress_), it(std::move(it_)), end(std::move(end_)) {
                initializeFromCurrentOutput();
            }
            
            
            Transaction read() const {
                assert(inTx.has_value() || outTx.has_value());
                if (outTx) {
                    return *outTx;
                } else {
                    return *inTx;
                }
            }
            
            bool equal(ranges::default_sentinel_t) const {
                return it == end;
            }
            
            void next() {
                if (inTx && outTx) {
                    outTx = ranges::nullopt;
                } else {
                    satisfyNext();
                }
            }
        };
        
        cursor begin_cursor() {
            return cursor{pointers.begin(), pointers.end(), searchAddress, access};
        }
        
        ranges::default_sentinel_t end_cursor() {
            return {};
        }
        
    public:
        AddressAllTxRange() = default;
        
        AddressAllTxRange(const Address &searchAddress_, DataAccess *access_) :
        access(access_), searchAddress(searchAddress_),
        pointers(access_->getAddressIndex().getOutputPointers(searchAddress_)
        | ranges::view::transform([](const InoutPointer &pointer) {
            return OutputPointer(pointer.txNum, pointer.inoutNum);
        })) {}
    };
    
    ranges::any_view<Transaction> Address::getTransactions() const {
        return AddressAllTxRange{*this, access};
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
        unsigned int nVersionBytes = access.config.chainConfig.pubkeyPrefix.size();
        CBitcoinAddress address{addressString, nVersionBytes};
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
                ss << int(script.getRequired()) << "of" << int(script.getTotal());
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

