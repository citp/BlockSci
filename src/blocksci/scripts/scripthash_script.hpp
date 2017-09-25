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
        
        ScriptType::Enum type() const override {
            return scriptType;
        }
        
        boost::optional<Transaction> transactionRevealed(const ChainAccess &chain) const;
        
        std::string addressString(const DataConfiguration &config) const;
        
        std::string toString(const DataConfiguration &config) const override;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const override;
        
        bool operator==(const Script &other) override;
        
        boost::optional<Address> getWrappedAddress() const;
        
        void visitPointers(const std::function<void(const Address &)> &visitFunc) const override {
            if (wrappedAddress.addressNum != 0) {
                visitFunc(wrappedAddress);
            }
        }
        
        
        std::unique_ptr<Script> wrappedScript(const ScriptAccess &access) const {
            return wrappedAddress.getScript(access);
        }
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        ScriptAddress<ScriptType::Enum::SCRIPTHASH>(uint32_t addressNum);
        std::string addressString() const;
        boost::optional<Transaction> transactionRevealed() const;
        std::unique_ptr<Script> wrappedScript() const;
        #endif
    };
}

#endif /* scripthash_script_hpp */
