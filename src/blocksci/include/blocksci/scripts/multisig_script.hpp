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

#include <blocksci/blocksci_export.h>
#include <blocksci/core/address_info.hpp>
#include <blocksci/util/data_access.hpp>

#include <sstream>

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
        
        std::string toPrettyString() const;

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
