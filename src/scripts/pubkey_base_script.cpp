//
//  pubkey_base_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//

#include <blocksci/scripts/pubkey_base_script.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/util/data_access.hpp>

namespace blocksci {
    ranges::any_view<Address> PubkeyAddressBase::getIncludingMultisigs() const {
        return getAccess().getAddressIndex().getIncludingMultisigs(*this);
    }
} // namespace blocksci
