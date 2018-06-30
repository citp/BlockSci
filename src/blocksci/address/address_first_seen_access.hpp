//
//  address_first_seen_access.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#ifndef address_first_seen_access_hpp
#define address_first_seen_access_hpp

#include "address_info.hpp"
#include <blocksci/file_mapper.hpp>

namespace blocksci {
    
    struct DataConfiguration;
    
    template<AddressType::Enum type>
    struct AddressFirstSeenFile : public FixedSizeFileMapper<uint32_t> {
        using FixedSizeFileMapper<uint32_t>::FixedSizeFileMapper;
    };
    
    class AddressFirstSeenAccess {
        using AddressFilesFirstSeenTuple = internal::to_script_type<AddressFirstSeenFile, AddressInfoList>::type;
        AddressFilesFirstSeenTuple addressFilesFirstSeen;
        
    public:
        
        AddressFirstSeenAccess(const DataConfiguration &config);
        
        template <AddressType::Enum type>
        uint32_t getFirstTxNum(uint32_t addressNum) const {
            auto &file = std::get<AddressFirstSeenFile<type>>(addressFilesFirstSeen);
            return *file.getData(addressNum - 1);
        }
    };
    
}

#endif /* address_first_seen_access_hpp */
