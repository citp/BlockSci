//
//  script_access.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include <blocksci/core/script_data.hpp>
#include <blocksci/util/hash.hpp>

namespace blocksci {
    uint160 PubkeyData::getPubkeyHash() const {
        if (hasPubkey) {
            return pubkey.GetID();
        } else {
            return address;
        }
    }
    
    uint160 ScriptHashData::getHash160() const {
        if (isSegwit) {
            return ripemd160(reinterpret_cast<const char *>(&hash256), sizeof(hash256));
        } else {
            return hash160;
        }
    }
} // namespace blocksci
