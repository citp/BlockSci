//
//  multisig_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "multisig_script.hpp"
#include "script_data.hpp"
#include "script_access.hpp"
#include "pubkey_script.hpp"
#include <blocksci/util/data_access.hpp>

namespace blocksci {
    using namespace script;
    
    Multisig::ScriptAddress(uint32_t addressNum_, const DataAccess &access) : ScriptBase(addressNum_, addressType, access), rawData(access.scripts->getScriptData<addressType>(addressNum_)) {}
    
    uint8_t Multisig::getRequired() const {
        return rawData->m;
    }
    
    uint8_t Multisig::getTotal() const {
        return rawData->n;
    }
    
    std::vector<Address> Multisig::getAddresses() const {
        std::vector<Address> addresses;
        addresses.reserve(rawData->addresses.size());
        for (auto scriptNum : rawData->addresses) {
            addresses.emplace_back(scriptNum, AddressType::Enum::MULTISIG_PUBKEY, getAccess());
        }
        return addresses;
    }
    
    std::string Multisig::toString() const {
        std::stringstream ss;
        ss << "MultisigAddress(" << static_cast<int>(getRequired()) << " of " << static_cast<int>(getTotal()) << ")";
        return ss.str();
    }
    
    std::string Multisig::toPrettyString() const {
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
    
    std::vector<script::MultisigPubkey> Multisig::pubkeyScripts() const {
        std::vector<script::MultisigPubkey> ret;
        ret.reserve(getTotal());
        for (auto &address : getAddresses()) {
            ret.emplace_back(address.scriptNum, getAccess());
        }
        return ret;
    }
}
