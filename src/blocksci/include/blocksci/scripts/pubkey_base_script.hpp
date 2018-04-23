//
//  pubkey_base_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef pubkey_base_script_h
#define pubkey_base_script_h

#include "script.hpp"

#include <blocksci/blocksci_export.h>
#include <blocksci/core/bitcoin_uint256.hpp>

namespace blocksci {
    class BLOCKSCI_EXPORT PubkeyAddressBase : public ScriptBase {
    protected:
        PubkeyAddressBase(uint32_t scriptNum_, AddressType::Enum type_, const PubkeyData *rawData_, DataAccess &access_) : ScriptBase(scriptNum_, type_, access_, rawData_) {}
        
        const PubkeyData *getData() const {
            return reinterpret_cast<const PubkeyData *>(ScriptBase::getData());
        }
        
    public:
        uint160 getPubkeyHash() const {
            return getData()->address;
        }
        
        ranges::optional<CPubKey> getPubkey() const {
            if (getData()->pubkey.IsValid()) {
                return getData()->pubkey;
            } else {
                return ranges::nullopt;
            }
        }
        
        std::vector<Address> getIncludingMultisigs() const;
    };
}

#endif /* pubkey_base_script_h */
