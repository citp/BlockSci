//
//  multisig_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/23/18.
//

#include <blocksci/scripts/multisig_script.hpp>
#include <blocksci/scripts/multisig_pubkey_script.hpp>

namespace blocksci {
    std::string ScriptAddress<AddressType::MULTISIG>::toPrettyString() const {
            std::stringstream ss;
            ss << "MultisigAddress(" << static_cast<int>(getRequired()) << " of " << static_cast<int>(getTotal()) << " multisig with addresses ";
            uint8_t i = 0;
            for (auto &address : getAddresses()) {
                script::MultisigPubkey pubkeyScript(address.scriptNum, getAccess());
                ss << pubkeyScript.toPrettyString();
                if (i < getTotal() - 1) {
                    ss << ", ";
                }
                i++;
            }
            ss << ")";
            return ss.str();
        }
} // namespace blocksci
