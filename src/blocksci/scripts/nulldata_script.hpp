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

#include <blocksci/util/data_access.hpp>

#include <string>
#include <sstream>

namespace blocksci {
    template <>
    class ScriptAddress<AddressType::NULL_DATA> : public ScriptBase {
        
        const RawData *getBackingData() const {
            return reinterpret_cast<const RawData *>(ScriptBase::getData());
        }
    public:
        constexpr static AddressType::Enum addressType = AddressType::NULL_DATA;
        
        ScriptAddress(uint32_t addressNum_, DataAccess &access_) : ScriptBase(addressNum_, addressType, access_, access_.scripts.getScriptData<dedupType(addressType)>(addressNum_)) {}
        
        std::string toString() const {
            std::stringstream ss;
            ss << "NulldataAddressData()";
            return ss.str();
        }
        
        std::string toPrettyString() const {
            std::stringstream ss;
            ss << "NulldataAddressData(" << getData() << ")";
            return ss.str();
        }
        
        std::string getData() const {
            return getBackingData()->getData();
        }
    };
} // namespace blocksci

#endif /* nulldata_script_hpp */
