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

namespace blocksci {
    using namespace script;
    
    Multisig::ScriptAddress(uint32_t scriptNum_, const MultisigData *rawData) : Script(scriptNum_), required(rawData->m), total(rawData->n), addresses(rawData->getAddresses()) {}
    
    Multisig::ScriptAddress(const ScriptAccess &access, uint32_t addressNum) : Multisig(addressNum, access.getScriptData<ScriptType::Enum::MULTISIG>(addressNum)) {}
    
    bool Multisig::operator==(const Script &other) {
        auto multisigOther = dynamic_cast<const Multisig *>(&other);
        
        if (!multisigOther) {
            return false;
        }
        
        if (multisigOther->required != required || multisigOther->addresses.size() != addresses.size()) {
            return false;
        }
        
        for (size_t i = 0; i < addresses.size(); i++) {
            if (addresses[i] != multisigOther->addresses[i]) {
                return false;
            }
        }
        
        return true;
    }
    
    std::string Multisig::toString(const DataConfiguration &) const {
        std::stringstream ss;
        ss << "MultisigAddress(required=" << static_cast<int>(required) << ", n=" << static_cast<int>(addresses.size()) << ", address_nums=[";
        uint32_t i = 0;
        for (auto &address : addresses) {
            ss << address.addressNum;
            if (i < addresses.size() - 1) {
                ss << ", ";
            }
            i++;
        }
        ss << "])";
        return ss.str();
    }
    
    std::string Multisig::toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const {
        std::stringstream ss;
        ss << static_cast<int>(required) << " of " << static_cast<int>(addresses.size()) << " multisig with addresses : [";
        uint32_t i = 0;
        for (auto &address : addresses) {
            ss << address.getScript(access)->toPrettyString(config, access);
            if (i < addresses.size() - 1) {
                ss << ", ";
            }
            i++;
        }
        ss << "]";
        return ss.str();
    }
}
