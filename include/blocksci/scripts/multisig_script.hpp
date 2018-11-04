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
#include "multisig_pubkey_script.hpp"

#include <blocksci/blocksci_export.h>

#include <range/v3/range_for.hpp>

#include <vector>

namespace blocksci {
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::MULTISIG> : public ScriptBase {
    private:
        const MultisigData *getData() const {
            return reinterpret_cast<const MultisigData *>(ScriptBase::getData());
        }
        
    public:
        constexpr static AddressType::Enum addressType = AddressType::MULTISIG;
        
        ScriptAddress(uint32_t addressNum_, const MultisigData *data_, DataAccess &access_) : ScriptBase(addressNum_, addressType, access_, data_) {}
        ScriptAddress(uint32_t addressNum_, DataAccess &access_);
        
        std::string toString() const;
        
        std::string toPrettyString() const;

        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            RANGES_FOR(auto address, getAddresses()) {
                visitFunc(address);
            }
        }
        
        int getRequired() const {
            return getData()->m;
        }
        
        int getTotal() const {
            return getData()->n;
        }
        
        ranges::any_view<Address, ranges::category::random_access | ranges::category::sized> getAddresses() const;
        ranges::any_view<script::MultisigPubkey, ranges::category::random_access | ranges::category::sized> pubkeyScripts() const;
    };
} // namespace blocksci

#endif /* multisig_script_hpp */
