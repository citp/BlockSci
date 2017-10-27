//
//  scripthash_script.hpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#ifndef scripthash_script_hpp
#define scripthash_script_hpp

#include "scriptsfwd.hpp"
#include "script.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/bitcoin_uint256.hpp>

namespace blocksci {
    struct ScriptHashData;
    class AnyScript;
    
    template <>
    class ScriptAddress<ScriptType::Enum::SCRIPTHASH> : public Script {
    private:
        Address wrappedAddress;
    public:
        uint32_t txRevealed;
        uint160 address;
        
        constexpr static ScriptType::Enum scriptType = ScriptType::Enum::SCRIPTHASH;
        
        ScriptAddress<scriptType>(uint32_t scriptNum, const ScriptHashData *rawData);
        ScriptAddress<scriptType>(const ScriptAccess &access, uint32_t addressNum);
        
        boost::optional<Transaction> transactionRevealed(const ChainAccess &chain) const;
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const;
        
        boost::optional<Address> getWrappedAddress() const;
        
        void visitPointers(const std::function<void(const Address &)> &visitFunc) const {
            if (wrappedAddress.scriptNum != 0) {
                visitFunc(wrappedAddress);
            }
        }
        
        
        AnyScript wrappedScript(const ScriptAccess &access) const;
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::SCRIPTHASH>(uint32_t addressNum);
        std::string addressString() const;
        boost::optional<Transaction> transactionRevealed() const;
        AnyScript wrappedScript() const;
        std::string toString() const;
        std::string toPrettyString() const;
        #endif
    };
}

#endif /* scripthash_script_hpp */
