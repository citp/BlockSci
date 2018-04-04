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

#include <blocksci/address/address.hpp>
#include <blocksci/blocksci_fwd.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/util/util.hpp>

#include <limits>

namespace blocksci {
    struct ScriptDataBase {
        uint32_t txFirstSeen;
        uint32_t txFirstSpent;
        
        explicit ScriptDataBase(uint32_t txNum) : txFirstSeen(txNum), txFirstSpent(std::numeric_limits<uint32_t>::max()) {}
    };
    
    struct PubkeyData : public ScriptDataBase {
        CPubKey pubkey;
        uint160 address;
        
        PubkeyData(uint32_t txNum, const CPubKey &pubkey_, uint160 address_) : ScriptDataBase(txNum), pubkey(pubkey_), address(address_) {}
        
        size_t size() {
            return sizeof(PubkeyData);
        }
    };
    
    struct ScriptHashData : public ScriptDataBase {
        union {
            uint160 hash160;
            uint256 hash256;
        };
        RawAddress wrappedAddress;
        bool isSegwit;
        
        ScriptHashData(uint32_t txNum, uint160 hash160_, Address wrappedAddress_) : ScriptDataBase(txNum), hash160(hash160_), wrappedAddress(wrappedAddress_), isSegwit(false) {}
        
        ScriptHashData(uint32_t txNum, uint256 hash256_, Address wrappedAddress_) : ScriptDataBase(txNum), hash256(hash256_), wrappedAddress(wrappedAddress_), isSegwit(true) {}
        
        size_t size() {
            return sizeof(ScriptHashData);
        }
        
        uint160 getHash160() const;
    };
    
    struct MultisigData : public ScriptDataBase {
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
    };
    
    struct NonstandardScriptData : public ScriptDataBase {
        InPlaceArray<unsigned char> scriptData;
        
        CScriptView getScript() const;
        
        size_t realSize() const {
            return sizeof(NonstandardScriptData) + scriptData.extraSize();
        }
        
        NonstandardScriptData(uint32_t txNum, uint32_t scriptLength) : ScriptDataBase(txNum), scriptData(scriptLength) {}
    };
    
    struct NonstandardSpendScriptData {
        InPlaceArray<unsigned char> scriptData;
        
        CScriptView getScript() const;
        
        size_t realSize() const {
            return sizeof(NonstandardScriptData) + scriptData.extraSize();
        }
        
        explicit NonstandardSpendScriptData(uint32_t scriptLength) : scriptData(scriptLength) {}
    };
    
    struct RawData : public ScriptDataBase {
        InPlaceArray<unsigned char> rawData;
        
        std::string getData() const {
            return std::string(rawData.begin(), rawData.end());
        }
        
        size_t realSize() const {
            return sizeof(RawData) + rawData.extraSize();
        }
        
        RawData(uint32_t txNum, const std::vector<unsigned char> &fullData) : ScriptDataBase(txNum), rawData(static_cast<uint32_t>(fullData.size())) {}
    };
} // namespace blocksci

#endif /* script_data_hpp */
