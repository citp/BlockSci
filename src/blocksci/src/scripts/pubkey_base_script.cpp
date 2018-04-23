//
//  pubkey_base_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//

#include <blocksci/scripts/pubkey_base_script.hpp>
#include <blocksci/util/data_access.hpp>

namespace blocksci {
    std::vector<Address> PubkeyAddressBase::getIncludingMultisigs() const {
        return getAccess().addressIndex.getIncludingMultisigs(*this);
    }
} // namespace blocksci
