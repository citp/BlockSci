//
//  address_first_seen_access.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/1/17.
//
//

#include "address_first_seen_access.hpp"
#include "data_configuration.hpp"

namespace blocksci {
    AddressFirstSeenAccess::AddressFirstSeenAccess(const DataConfiguration &config) :
    addressFilesFirstSeen(blocksci::apply(blocksci::AddressInfoList(), [&] (auto tag) {
        return config.firstSeenDirectory()/blocksci::AddressInfo<decltype(tag)::type>::typeName;
    })) {}
}
