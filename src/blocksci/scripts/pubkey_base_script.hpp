//
//  pubkey_base_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef pubkey_base_script_h
#define pubkey_base_script_h

#include "script.hpp"
#include "script_access.hpp"

#include <blocksci/index/address_index.hpp>

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    class PubkeyAddressBase : public ScriptBase<PubkeyAddressBase> {
        friend class ScriptBase<PubkeyAddressBase>;
        const PubkeyData *rawData;
        
    protected:
        PubkeyAddressBase(uint32_t scriptNum_, AddressType::Enum type_, const PubkeyData *rawData_, DataAccess &access_) : ScriptBase(scriptNum_, type_, access_), rawData(rawData_) {}
        
    public:
        uint160 getPubkeyHash() const {
            return rawData->address;
        }
        
        ranges::optional<CPubKey> getPubkey() const {
            if (rawData->pubkey.IsValid()) {
                return rawData->pubkey;
            } else {
                return ranges::nullopt;
            }
        }
        
        std::vector<Address> getIncludingMultisigs() const {
            return getAccess().addressIndex.getIncludingMultisigs(*this);
        }
    };
}

#endif /* pubkey_base_script_h */
