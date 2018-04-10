//
//  multisig_pubkey_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#include "multisig_pubkey_script.hpp"
#include "bitcoin_base58.hpp"

namespace blocksci {
    using script::MultisigPubkey;
    
    std::string MultisigPubkey::addressString() const {
        return CBitcoinAddress(getPubkeyHash(), AddressType::Enum::MULTISIG_PUBKEY, getAccess().config).ToString();
    }
} // namespace blocksci
