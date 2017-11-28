//
//  scripthash_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef scripthash_script_hpp
#define scripthash_script_hpp

#include "script.hpp"
#include "scripts_fwd.hpp"
#include <blocksci/address/address.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>

#include <range/v3/utility/optional.hpp>

namespace blocksci {
    template <>
    class ScriptAddress<ScriptType::Enum::SCRIPTHASH> : public BaseScript {
    private:
        Address wrappedAddress;
    public:
        uint160 address;
        
        constexpr static ScriptType::Enum scriptType = ScriptType::Enum::SCRIPTHASH;
        
        ScriptAddress<scriptType>(uint32_t scriptNum, const ScriptHashData *rawData, const ScriptAccess &access);
        ScriptAddress<scriptType>(const ScriptAccess &access, uint32_t addressNum);
        
        ranges::optional<Transaction> transactionRevealed(const ChainAccess &chain) const;
        
        std::string addressString() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
        
        ranges::optional<Address> getWrappedAddress() const;
        
        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            if (hasBeenSpent()) {
                visitFunc(wrappedAddress);
            }
        }
        
        AnyScript wrappedScript() const;
    };
}

#endif /* scripthash_script_hpp */
