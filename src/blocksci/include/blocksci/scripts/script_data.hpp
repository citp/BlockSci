//
//  script_data.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/17/17.
//
//

#ifndef script_data_hpp
#define script_data_hpp

#include "bitcoin_pubkey.hpp"

#include <blocksci/address/raw_address.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/util/hash.hpp>
#include <blocksci/util/in_place_array.hpp>

#include <range/v3/utility/optional.hpp>

#include <limits>

namespace blocksci {
    
    struct BLOCKSCI_EXPORT ScriptDataBase {
        uint32_t txFirstSeen;
        uint32_t txFirstSpent;
        
        explicit ScriptDataBase(uint32_t txNum) : txFirstSeen(txNum), txFirstSpent(std::numeric_limits<uint32_t>::max()) {}
        
        void visitPointers(const std::function<void(const RawAddress &)> &) const {}
        
        uint32_t getFirstTxIndex() const {
            return txFirstSeen;
        }
        
        ranges::optional<uint32_t> getTxRevealedIndex() const {
            if (txFirstSpent != std::numeric_limits<uint32_t>::max()) {
                return txFirstSpent;
            } else {
                return ranges::nullopt;
            }
        }
        
        bool hasBeenSpent() const {
            return getTxRevealedIndex().has_value();
        }
    };
    
    struct BLOCKSCI_EXPORT PubkeyData : public ScriptDataBase {
        CPubKey pubkey;
        uint160 address;
        
        PubkeyData(uint32_t txNum, const CPubKey &pubkey_, uint160 address_) : ScriptDataBase(txNum), pubkey(pubkey_), address(address_) {}
        
        size_t size() {
            return sizeof(PubkeyData);
        }
    };
    
    struct BLOCKSCI_EXPORT ScriptHashData : public ScriptDataBase {
        union {
            uint160 hash160;
            uint256 hash256;
        };
        RawAddress wrappedAddress;
        bool isSegwit;
        
        ScriptHashData(uint32_t txNum, uint160 hash160_, const RawAddress &wrappedAddress_) : ScriptDataBase(txNum), hash160(hash160_), wrappedAddress(wrappedAddress_), isSegwit(false) {}
        
        ScriptHashData(uint32_t txNum, uint256 hash256_, const RawAddress &wrappedAddress_) : ScriptDataBase(txNum), hash256(hash256_), wrappedAddress(wrappedAddress_), isSegwit(true) {}
        
        size_t size() {
            return sizeof(ScriptHashData);
        }
        
        ranges::optional<RawAddress> getWrappedAddress() const {
            if (wrappedAddress.scriptNum != 0) {
                return wrappedAddress;
            } else {
                return ranges::nullopt;
            }
        }
        
        void visitPointers(const std::function<void(const RawAddress &)> &visitFunc) const {
            auto wrapped = getWrappedAddress();
            if (wrapped) {
                visitFunc(*wrapped);
            }
        }
        
        
        uint160 getHash160() const {
            if (isSegwit) {
                return ripemd160(reinterpret_cast<const char *>(&hash256), sizeof(hash256));
            } else {
                return hash160;
            }
        }
    };
    
    struct BLOCKSCI_EXPORT MultisigData : public ScriptDataBase {
        uint8_t m;
        uint8_t n;
        InPlaceArray<uint32_t, uint16_t> addresses;
        
        MultisigData(const MultisigData &other) = delete;
        MultisigData(MultisigData &&other) = delete;
        MultisigData &operator=(const MultisigData &other) = delete;
        MultisigData &operator=(MultisigData &&other) = delete;
                
        size_t realSize() const {
            return sizeof(MultisigData) + addresses.extraSize();
        }
        
        MultisigData(uint32_t txNum, uint8_t m_, uint8_t n_, uint16_t addressCount) : ScriptDataBase(txNum), m(m_), n(n_), addresses(addressCount) {}
        
        void visitPointers(const std::function<void(const RawAddress &)> &visitFunc) const {
            for (auto scriptNum : addresses) {
                visitFunc(RawAddress{scriptNum, AddressType::Enum::MULTISIG_PUBKEY});
            }
        }
    };
    
    struct BLOCKSCI_EXPORT NonstandardScriptData : public ScriptDataBase {
        InPlaceArray<unsigned char> scriptData;
        
        size_t realSize() const {
            return sizeof(NonstandardScriptData) + scriptData.extraSize();
        }
        
        NonstandardScriptData(uint32_t txNum, uint32_t scriptLength) : ScriptDataBase(txNum), scriptData(scriptLength) {}
    };
    
    struct BLOCKSCI_EXPORT NonstandardSpendScriptData {
        InPlaceArray<unsigned char> scriptData;
        
        size_t realSize() const {
            return sizeof(NonstandardScriptData) + scriptData.extraSize();
        }
        
        explicit NonstandardSpendScriptData(uint32_t scriptLength) : scriptData(scriptLength) {}
    };
    
    struct BLOCKSCI_EXPORT RawData : public ScriptDataBase {
        InPlaceArray<unsigned char> rawData;
        
        std::string getData() const {
            return std::string(rawData.begin(), rawData.end());
        }
        
        size_t realSize() const {
            return sizeof(RawData) + rawData.extraSize();
        }
        
        RawData(uint32_t txNum, const std::vector<unsigned char> &fullData) : ScriptDataBase(txNum), rawData(static_cast<uint32_t>(fullData.size())) {}
    };

    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::PUBKEY> {
        using type = PubkeyData;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::PUBKEYHASH> {
        using type = PubkeyData;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::MULTISIG_PUBKEY> {
        using type = PubkeyData;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::WITNESS_PUBKEYHASH> {
        using type = PubkeyData;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::SCRIPTHASH> {
        using type = ScriptHashData;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::WITNESS_SCRIPTHASH> {
        using type = ScriptHashData;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::MULTISIG> {
        using type = MultisigData;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::NULL_DATA> {
        using type = RawData;
    };
    
    template <>
    struct BLOCKSCI_EXPORT ScriptData<AddressType::NONSTANDARD> {
        using type = NonstandardScriptData;
    };
    
} // namespace blocksci

#endif /* script_data_hpp */
