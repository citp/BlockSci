//
//  multisig_pubkey_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "bitcoin_base58.hpp"

#include <blocksci/scripts/multisig_pubkey_script.hpp>

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>

#include <sstream>

namespace blocksci {
    ScriptAddress<AddressType::MULTISIG_PUBKEY>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    std::string ScriptAddress<AddressType::MULTISIG_PUBKEY>::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::MULTISIG_PUBKEY, getAccess().config).ToString();
    }
    
    std::string ScriptAddress<AddressType::MULTISIG_PUBKEY>::toString() const {
        std::stringstream ss;
        ss << "MultisigPubkeyAddress(" << addressString() << ")";
        return ss.str();
    }
} // namespace blocksci
