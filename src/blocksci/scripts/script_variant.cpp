//
//  script_variant.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/14/17.
//

#include "script_variant.hpp"
#include <blocksci/chain/transaction.hpp>
#include <blocksci/address/address_info.hpp>

namespace blocksci {
    
    template<AddressType::Enum type>
    struct ScriptCreateFunctor {
        static AnyScript::ScriptVariant f(uint32_t scriptNum, const DataAccess &access) {
            return ScriptAddress<type>(scriptNum, access);
        }
    };
    
    static constexpr auto scriptCreator = make_dynamic_table<AddressType, ScriptCreateFunctor>();
    
    AnyScript::AnyScript(const Address &address, const DataAccess &access) : wrapped(scriptCreator.at(static_cast<size_t>(address.type))(address.scriptNum, access)) {}
    
    AnyScript::AnyScript(uint32_t addressNum, AddressType::Enum type, const DataAccess &access) : wrapped(scriptCreator.at(static_cast<size_t>(type))(addressNum, access)) {}
    
    AddressType::Enum AnyScript::type() const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.addressType; }, wrapped);
    }
    
    void AnyScript::visitPointers(const std::function<void(const Address &)> &func) {
        mpark::visit([&](auto &scriptAddress) { scriptAddress.visitPointers(func); }, wrapped);
    }
    
    uint32_t AnyScript::firstTxIndex() {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getFirstTxIndex(); }, wrapped);
    }
    
    uint32_t AnyScript::txRevealedIndex() {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getTxRevealedIndex(); }, wrapped);
    }
    
    Transaction AnyScript::getFirstTransaction() const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getFirstTransaction(); }, wrapped);
    }
    
    ranges::optional<Transaction> AnyScript::getTransactionRevealed() const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getTransactionRevealed(); }, wrapped);
    }
    
    std::string AnyScript::toString() const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.toString(); }, wrapped);
    }
    
    std::string AnyScript::toPrettyString() const {
        return mpark::visit([&](auto &scriptAddress) { return scriptAddress.toPrettyString(); }, wrapped);
    }
}
