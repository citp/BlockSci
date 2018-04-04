//
//  nulldata_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef nulldata_script_hpp
#define nulldata_script_hpp

#include "script.hpp"

#include <string>

namespace blocksci {
    template <>
    class ScriptAddress<AddressType::NULL_DATA> : public ScriptBase<ScriptAddress<AddressType::NULL_DATA>> {
        friend class ScriptBase<ScriptAddress<AddressType::NULL_DATA>>;
        const RawData *rawData;
    public:
        constexpr static AddressType::Enum addressType = AddressType::NULL_DATA;
        
        ScriptAddress(uint32_t addressNum_, const DataAccess &access_);
        
        std::string toString() const;
        std::string toPrettyString() const;
        
        std::string getData() const;
    };
} // namespace blocksci

#endif /* nulldata_script_hpp */
