//
//  script_variant.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/14/17.
//

#include "script_variant.hpp"
#include "chain/transaction.hpp"
#include <blocksci/address/address_info.hpp>

namespace blocksci {
    
    template<AddressType::Enum type>
    struct ScriptCreateFunctor {
        static AnyScript::ScriptVariant f(const ScriptAccess &access, uint32_t scriptNum) {
            return ScriptAddress<type>(access, scriptNum);
        }
    };
    
    static constexpr auto scriptCreator = make_dynamic_table<AddressType, ScriptCreateFunctor>();
    
    AnyScript::AnyScript(const Address &address, const ScriptAccess &access) : wrapped(scriptCreator[static_cast<size_t>(address.type)](access, address.scriptNum)) {}
    
    AddressType::Enum AnyScript::type() const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.addressType; }, wrapped);
    }
    
    void AnyScript::visitPointers(const std::function<void(const Address &)> &func) {
        mpark::visit([&](auto &scriptAddress) { scriptAddress.visitPointers(func); }, wrapped);
    }
    
    uint32_t AnyScript::firstTxIndex() {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.firstTxIndex; }, wrapped);
    }
    
    uint32_t AnyScript::txRevealedIndex() {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.txRevealed; }, wrapped);
    }
    
    Transaction AnyScript::getFirstTransaction(const ChainAccess &chain) const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getFirstTransaction(chain); }, wrapped);
    }
    
    ranges::optional<Transaction> AnyScript::getTransactionRevealed(const ChainAccess &chain) const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getTransactionRevealed(chain); }, wrapped);
    }
    
    std::string AnyScript::toPrettyString() const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.toPrettyString(); }, wrapped);
    }
}
