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
    
    Multisig::ScriptAddress(uint32_t scriptNum_, const MultisigData *rawData, const DataAccess &access) : Script(scriptNum_, addressType, *rawData, access), required(rawData->m), total(rawData->n) {
        addresses.reserve(rawData->addresses.size());
        for (auto scriptNum : rawData->addresses) {
            addresses.emplace_back(scriptNum, AddressType::Enum::PUBKEYHASH, access);
        }
    }
    
    Multisig::ScriptAddress(uint32_t addressNum, const DataAccess &access) : Multisig(addressNum, access.scripts->getScriptData<addressType>(addressNum), access) {}
    
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
            script::MultisigPubkey pubkeyScript(address.scriptNum, *access);
            ss << pubkeyScript.toPrettyString();
            if (i < addresses.size() - 1) {
                ss << ", ";
            }
            i++;
        }
        ss << "]";
        return ss.str();
    }
    
    std::vector<script::MultisigPubkey> Multisig::pubkeyScripts() const {
        std::vector<script::MultisigPubkey> ret;
        ret.reserve(addresses.size());
        for (auto &address : addresses) {
            ret.emplace_back(address.scriptNum, *access);
        }
        return ret;
    }
}
