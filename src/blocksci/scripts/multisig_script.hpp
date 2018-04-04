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

namespace blocksci {
    template <>
    class ScriptAddress<AddressType::MULTISIG> : public ScriptBase<ScriptAddress<AddressType::MULTISIG>> {
        friend class ScriptBase<ScriptAddress<AddressType::MULTISIG>>;
        const MultisigData *rawData;
    public:
        constexpr static AddressType::Enum addressType = AddressType::MULTISIG;
        ScriptAddress(uint32_t addressNum_, const DataAccess &access_);
        
        std::string toString() const;
        std::string toPrettyString() const;

        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            for (auto &address : getAddresses()) {
                visitFunc(address);
            }
        }
        
        uint8_t getRequired() const;
        uint8_t getTotal() const;
        std::vector<Address> getAddresses() const;
        
        std::vector<script::MultisigPubkey> pubkeyScripts() const;
    };
} // namespace blocksci

#endif /* multisig_script_hpp */
