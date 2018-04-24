//
//  multisig_pubkey_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include <blocksci/scripts/multisig_pubkey_script.hpp>

#include "bitcoin_base58.hpp"

namespace blocksci {
    std::string ScriptAddress<AddressType::MULTISIG_PUBKEY>::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::MULTISIG_PUBKEY, getAccess().config).ToString();
    }
} // namespace blocksci
