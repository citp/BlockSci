//
//  pubkey_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include <blocksci/scripts/pubkey_script.hpp>

#include "bitcoin_base58.hpp"
#include "bitcoin_segwit_addr.hpp"

namespace blocksci {
    std::string ScriptAddress<AddressType::PUBKEY>::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::PUBKEYHASH, getAccess().config).ToString();
    }
    
    std::string ScriptAddress<AddressType::PUBKEYHASH>::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::PUBKEYHASH, getAccess().config).ToString();
    }
    
    std::string ScriptAddress<AddressType::WITNESS_PUBKEYHASH>::addressString() const {
        std::vector<uint8_t> witprog;
        auto pubkeyhash = getPubkeyHash();
        witprog.insert(witprog.end(), reinterpret_cast<const uint8_t *>(&pubkeyhash), reinterpret_cast<const uint8_t *>(&pubkeyhash) + sizeof(pubkeyhash));
        return segwit_addr::encode(getAccess().config, 0, witprog);
    }
} // namespace blocksci
