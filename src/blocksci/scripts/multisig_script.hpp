//
//  multisig_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef multisig_script_hpp
#define multisig_script_hpp

#include <blocksci/blocksci_export.h>
#include "script.hpp"
#include "multisig_pubkey_script.hpp"

namespace blocksci {
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::MULTISIG> : public ScriptBase {
    private:
        const MultisigData *getData() const {
            return reinterpret_cast<const MultisigData *>(ScriptBase::getData());
        }
    public:
        constexpr static AddressType::Enum addressType = AddressType::MULTISIG;
        ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptBase(addressNum_, addressType, access_, access_.scripts.getScriptData<dedupType(addressType)>(addressNum_)) {}
        
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
            return getData()->m;
        }
        
        uint8_t getTotal() const {
            return getData()->n;
        }
        
        std::vector<Address> getAddresses() const {
            std::vector<Address> addresses;
            addresses.reserve(getData()->addresses.size());
            for (auto scriptNum : getData()->addresses) {
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
