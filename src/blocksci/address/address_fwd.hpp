//
//  address_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/8/17.
//

#ifndef address_fwd_h
#define address_fwd_h

#include "address_types.hpp"
#include "dedup_address_type.hpp"

namespace blocksci {
    struct Address;
    struct DedupAddress;
    
    template <AddressType::Enum>
    struct AddressInfo;
    
    template <DedupAddressType::Enum>
    struct DedupAddressInfo;
}

#endif /* address_fwd_h */
