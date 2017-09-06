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

#include <blocksci/address/address.hpp>

#include <blocksci/bitcoin_uint256.hpp>

#include <stdio.h>

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
        
        size_t size() {
            return sizeof(ScriptHashData);
        }
    };
    
    struct MultisigData {
        uint8_t m;
        uint8_t n;
        uint16_t addressCount;
        
        MultisigData(const MultisigData &other) = delete;
        MultisigData(MultisigData &&other) = delete;
        MultisigData &operator=(const MultisigData &other) = delete;
        MultisigData &operator=(MultisigData &&other) = delete;
        
        std::vector<Address> getAddresses() const {
            std::vector<Address> res;
            
            const uint32_t *addresses = reinterpret_cast<const uint32_t *>(reinterpret_cast<const char *>(this) + sizeof(MultisigData));
            for (uint32_t i = 0; i < addressCount; i++) {
                res.emplace_back(addresses[i], AddressType::Enum::PUBKEYHASH);
            }
            
            return res;
        }
        
        size_t size() {
            return sizeof(MultisigData) + sizeof(uint32_t) * addressCount;
        }
    };
    
    struct NonstandardScriptData {
        uint32_t length;
        
        CScript getScript() const {
            const unsigned char *start = reinterpret_cast<const unsigned char *>(this) + sizeof(NonstandardScriptData);
            return CScript(start, start + length);
        }
        
        size_t size() {
            return sizeof(NonstandardScriptData) + length;
        }
    };
    
    struct RawData {
        uint32_t length;
        
        std::string getData() const {
            const char *start = reinterpret_cast<const char *>(this) + sizeof(RawData);
            return std::string(start, length);
        }
        
        size_t size() {
            return sizeof(RawData) + length;
        }
    };
}


#endif /* script_data_hpp */
