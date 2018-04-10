//
//  multisig_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef multisig_script_hpp
#define multisig_script_hpp

#include "script.hpp"
#include "script_access.hpp"
#include "multisig_pubkey_script.hpp"

namespace blocksci {
    template <>
    class ScriptAddress<AddressType::MULTISIG> : public ScriptBase<ScriptAddress<AddressType::MULTISIG>> {
        friend class ScriptBase<ScriptAddress<AddressType::MULTISIG>>;
        const MultisigData *rawData;
    public:
        constexpr static AddressType::Enum addressType = AddressType::MULTISIG;
        ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptBase(addressNum_, addressType, access_), rawData(access_.scripts.getScriptData<addressType>(addressNum_)) {}
        
        std::string toString() const {
            std::stringstream ss;
            ss << "MultisigAddress(" << static_cast<int>(getRequired()) << " of " << static_cast<int>(getTotal()) << ")";
            return ss.str();
        }
        
        std::string toPrettyString() const {
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

        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            for (auto &address : getAddresses()) {
                visitFunc(address);
            }
        }
        
        uint8_t getRequired() const {
            return rawData->m;
        }
        
        uint8_t getTotal() const {
            return rawData->n;
        }
        
        std::vector<Address> getAddresses() const {
            std::vector<Address> addresses;
            addresses.reserve(rawData->addresses.size());
            for (auto scriptNum : rawData->addresses) {
                addresses.emplace_back(scriptNum, AddressType::Enum::MULTISIG_PUBKEY, getAccess());
            }
            return addresses;
        }
        
        std::vector<script::MultisigPubkey> pubkeyScripts() const {
            std::vector<script::MultisigPubkey> ret;
            ret.reserve(getTotal());
            for (auto &address : getAddresses()) {
                ret.emplace_back(address.scriptNum, getAccess());
            }
            return ret;
        }
    };
} // namespace blocksci

#endif /* multisig_script_hpp */
