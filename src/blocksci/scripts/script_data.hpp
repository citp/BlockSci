//
//  script_data.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/17/17.
//
//

#ifndef script_data_hpp
#define script_data_hpp

#include "bitcoin_script.hpp"
#include "bitcoin_pubkey.hpp"
#include "scriptsfwd.hpp"

#include <blocksci/util.hpp>
#include <blocksci/typedefs.hpp>
#include <blocksci/file_mapper.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/bitcoin_uint256.hpp>

namespace blocksci {
    
    struct PubkeyData {
        CPubKey pubkey;
        uint160 address;
        
        size_t size() {
            return sizeof(PubkeyData);
        }
    };
    
    struct ScriptHashData {
        uint160 address;
        Address wrappedAddress;
        TxIndex txRevealed;
        
        ScriptHashData(uint160 address_, Address wrappedAddress_, TxIndex txRevealed_) : address(address_), wrappedAddress(wrappedAddress_), txRevealed(txRevealed_) {}
        
        size_t size() {
            return sizeof(ScriptHashData);
        }
    };
    
    struct MultisigData {
        uint8_t m;
        uint8_t n;
        InPlaceArray<uint32_t, uint16_t> addresses;
        
        MultisigData(const MultisigData &other) = delete;
        MultisigData(MultisigData &&other) = delete;
        MultisigData &operator=(const MultisigData &other) = delete;
        MultisigData &operator=(MultisigData &&other) = delete;
        
        std::vector<Address> getAddresses() const {
            std::vector<Address> res;
            res.reserve(addresses.size());
            for (auto scriptNum : addresses) {
                res.emplace_back(scriptNum, AddressType::Enum::PUBKEYHASH);
            }
            
            return res;
        }
        
        size_t realSize() const {
            return sizeof(MultisigData) + addresses.extraSize();
        }
        
        MultisigData(uint8_t m_, uint8_t n_, uint16_t addressCount) : m(m_), n(n_), addresses(addressCount) {}
    };
    
    struct NonstandardScriptData {
        InPlaceArray<unsigned char> scriptData;
        
        CScript getScript() const {
            return CScript(scriptData.begin(), scriptData.end());
        }
        
        size_t realSize() const {
            return sizeof(NonstandardScriptData) + scriptData.extraSize();
        }
        
        NonstandardScriptData(const CScriptView &scriptView);
        NonstandardScriptData(const CScript &scriptView);
    };
    
    struct RawData {
        InPlaceArray<unsigned char> rawData;
        
        std::string getData() const {
            return std::string(rawData.begin(), rawData.end());
        }
        
        size_t realSize() const {
            return sizeof(RawData) + rawData.extraSize();
        }
        
        RawData(const std::vector<unsigned char> &fullData) : rawData(fullData.size()) {}
    };
    
    
}


#endif /* script_data_hpp */
