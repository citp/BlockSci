//
//  script_data.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/17/17.
//
//

#ifndef script_data_hpp
#define script_data_hpp

#include "address_pointer.hpp"
#include "script.hpp"
#include "pubkey.hpp"

#include <blocksci/uint256.hpp>

#include <stdio.h>

namespace blocksci {
    struct PubkeyData {
        CPubKey pubkey;
        
        size_t size() {
            return sizeof(pubkey);
        }
    };
    
    struct PubkeyHashData {
        uint160 address;
        
        size_t size() {
            return sizeof(PubkeyHashData);
        }
    };
    
    struct ScriptHashData {
        uint160 address;
        AddressPointer wrappedAddress;
        
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
        
        std::vector<AddressPointer> getAddresses() const {
            std::vector<AddressPointer> res;
            
            const uint32_t *addresses = reinterpret_cast<const uint32_t *>(reinterpret_cast<const char *>(this) + sizeof(MultisigData));
            for (uint32_t i = 0; i < addressCount; i++) {
                res.emplace_back(addresses[i], ScriptType::Enum::PUBKEYHASH);
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
