//
//  raw_address.hpp
//  bitcoinapi
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef raw_address_hpp
#define raw_address_hpp

#include <blocksci/scripts/scripts_fwd.hpp>

#include <functional>

namespace blocksci {
    struct RawAddress {
        uint32_t scriptNum;
        AddressType::Enum type;
        
        RawAddress() {}
        RawAddress(uint32_t addressNum_, AddressType::Enum type_) : scriptNum(addressNum_), type(type_) {}
    };
    
    void visit(const RawAddress &address, const std::function<bool(const RawAddress &)> &visitFunc, const ScriptAccess &scripts);
} // namespace blocksci

#endif /* raw_address_hpp */
