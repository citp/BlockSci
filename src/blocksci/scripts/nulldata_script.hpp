//
//  nulldata_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef nulldata_script_hpp
#define nulldata_script_hpp

#include "scriptsfwd.hpp"
#include "script.hpp"
#include "bitcoin_script.hpp"

namespace blocksci {
    struct RawData;
    
    template <>
    class ScriptAddress<AddressType::Enum::NULL_DATA> : public Script {
        
    public:
        ScriptAddress<AddressType::Enum::NULL_DATA>(const RawData *rawData);
        ScriptAddress<AddressType::Enum::NULL_DATA>(const ScriptAccess &access, uint32_t addressNum);
        std::string data;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        bool operator==(const Script &other) override;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<AddressType::Enum::NULL_DATA>(uint32_t addressNum);
        #endif
    };
}

#endif /* nulldata_script_hpp */
