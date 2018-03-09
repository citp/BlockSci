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
    class ScriptAddress<AddressType::MULTISIG> : public Script {
        
    public:
        constexpr static AddressType::Enum addressType = AddressType::MULTISIG;
        uint8_t required;
        uint8_t total;
        std::vector<Address> addresses;
        
        ScriptAddress(uint32_t scriptNum, const MultisigData *rawData, const ScriptAccess &access);
        ScriptAddress(const ScriptAccess &access, uint32_t addressNum);
        
        
        std::string toString() const;
        std::string toPrettyString() const;

        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            for (auto &address : addresses) {
                visitFunc(address);
            }
        }
        
        std::vector<ScriptAddress<AddressType::PUBKEY>> pubkeyScripts() const;
    };
}


#endif /* multisig_script_hpp */
