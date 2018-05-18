//
//  multisig_script.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/23/18.
//

#include <blocksci/scripts/multisig_script.hpp>
#include <blocksci/scripts/multisig_pubkey_script.hpp>

#include <internal/address_info.hpp>
#include <internal/data_access.hpp>
#include <internal/script_access.hpp>

#include <sstream>

namespace blocksci {
    ScriptAddress<AddressType::MULTISIG>::ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptAddress(addressNum_, access_.getScripts().getScriptData<dedupType(addressType)>(addressNum_), access_) {}
    
    std::string ScriptAddress<AddressType::MULTISIG>::toString() const {
        std::stringstream ss;
        ss << "MultisigAddress(" << static_cast<int>(getRequired()) << " of " << static_cast<int>(getTotal()) << ")";
        return ss.str();
    }
    
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
