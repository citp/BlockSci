//
//  raw_address_pointer_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/31/17.
//
//

#include "raw_address_pointer.hpp"
#include "data_access.hpp"

namespace blocksci {
    boost::optional<RawAddress> RawAddress::create(const std::string &addressString) {
        return create(DataAccess::Instance().config, addressString);
    }
}
