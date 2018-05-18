//
//  pubkey_base_script.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//

#include <blocksci/scripts/pubkey_base_script.hpp>
#include <blocksci/scripts/bitcoin_pubkey.hpp>

#include <internal/address_index.hpp>
#include <internal/data_access.hpp>

#include <range/v3/view/transform.hpp>

namespace blocksci {
    
    ranges::optional<CPubKey> PubkeyAddressBase::getPubkey() const {
        auto &rawPubkey = getData()->pubkey;
        CPubKey pubkey{rawPubkey.begin(), rawPubkey.end()};
        if (pubkey.IsValid()) {
            return pubkey;
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::any_view<Address> PubkeyAddressBase::getIncludingMultisigs() const {
        auto access_ = &getAccess();
        return getAccess().getAddressIndex().getIncludingMultisigs(*this) |
        ranges::view::transform([access_](const RawAddress &raw) { return Address{raw, *access_}; });
    }
} // namespace blocksci
