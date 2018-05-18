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

#include <blocksci/blocksci_export.h>

#include <string>

namespace blocksci {
    template <>
    class BLOCKSCI_EXPORT ScriptAddress<AddressType::NULL_DATA> : public ScriptBase {
        
        const RawData *getBackingData() const {
            return reinterpret_cast<const RawData *>(ScriptBase::getData());
        }
        
    public:
        constexpr static AddressType::Enum addressType = AddressType::NULL_DATA;
        
        ScriptAddress(uint32_t addressNum_, const RawData *data_, DataAccess &access_) : ScriptBase(addressNum_, addressType, access_, data_) {}
        ScriptAddress(uint32_t addressNum_, DataAccess &access_);
        
        std::string toString() const {
            return "NulldataAddressData()";
        }
        
        std::string toPrettyString() const;
        
        std::string getData() const {
            return getBackingData()->getData();
        }
        
        bool isSegwitMarker() const;
    };
} // namespace blocksci

#endif /* nulldata_script_hpp */
