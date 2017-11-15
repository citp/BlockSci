//
//  multisig_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef multisig_script_hpp
#define multisig_script_hpp

#include "scriptsfwd.hpp"
#include "script.hpp"

namespace blocksci {
    struct MultisigData;
    
    template <>
    class ScriptAddress<ScriptType::Enum::MULTISIG> : public BaseScript {
        
    public:
        constexpr static ScriptType::Enum scriptType = ScriptType::Enum::MULTISIG;
        uint8_t required;
        uint8_t total;
        std::vector<Address> addresses;
        
        ScriptAddress<scriptType>(uint32_t scriptNum, const MultisigData *rawData);
        ScriptAddress<scriptType>(const ScriptAccess &access, uint32_t addressNum);
        
        
        std::string toString(const DataConfiguration &config) const;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const;

        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            for (auto &address : addresses) {
                visitFunc(address);
            }
        }
        
#ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<scriptType>(uint32_t addressNum);
        std::string toString() const;
        std::string toPrettyString() const;
#endif
    };
}


#endif /* multisig_script_hpp */
