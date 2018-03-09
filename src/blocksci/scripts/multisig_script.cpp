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

namespace blocksci {
    using namespace script;
    
    Multisig::ScriptAddress(uint32_t scriptNum_, const MultisigData *rawData, const ScriptAccess &access) : Script(scriptNum_, addressType, *rawData, access), required(rawData->m), total(rawData->n), addresses(rawData->getAddresses()) {}
    
    Multisig::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Multisig(addressNum, access.getScriptData<addressType>(addressNum), access) {}
    
    std::string Multisig::toString() const {
        std::stringstream ss;
        ss << "MultisigAddress(required=" << static_cast<int>(required) << ", n=" << static_cast<int>(addresses.size()) << ", address_nums=[";
        uint32_t i = 0;
        for (auto &address : addresses) {
            ss << address.scriptNum;
            if (i < addresses.size() - 1) {
                ss << ", ";
            }
            i++;
        }
        ss << "])";
        return ss.str();
    }
    
    std::string Multisig::toPrettyString() const {
        std::stringstream ss;
        ss << static_cast<int>(required) << " of " << static_cast<int>(addresses.size()) << " multisig with addresses : [";
        uint32_t i = 0;
        for (auto &address : addresses) {
            script::Pubkey pubkeyScript(*access, address.scriptNum);
            ss << pubkeyScript.toPrettyString();
            if (i < addresses.size() - 1) {
                ss << ", ";
            }
            i++;
        }
        ss << "]";
        return ss.str();
    }
    
    std::vector<ScriptAddress<AddressType::Enum::PUBKEY>> Multisig::pubkeyScripts() const {
        std::vector<ScriptAddress<AddressType::Enum::PUBKEY>> ret;
        ret.reserve(addresses.size());
        for (auto &address : addresses) {
            ret.emplace_back(*access, address.scriptNum);
        }
        return ret;
    }
}
