//
//  script_variant.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/14/17.
//

#ifndef script_variant_hpp
#define script_variant_hpp

#include "multisig_script.hpp"
#include "nonstandard_script.hpp"
#include "nulldata_script.hpp"
#include "pubkey_script.hpp"
#include "multisig_pubkey_script.hpp"
#include "scripthash_script.hpp"
#include "witness_unknown_script.hpp"

#include <blocksci/blocksci_export.h>

#include <mpark/variant.hpp>

namespace blocksci {
    class BLOCKSCI_EXPORT AnyScript {
    public:
        AnyScript() = default;
        AnyScript(const Address &address);
        AnyScript(uint32_t addressNum, AddressType::Enum type, DataAccess &access);
        AnyScript(const ScriptVariant &var) : wrapped(var) {}

        uint32_t getScriptNum() const {
            return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getScriptNum(); }, wrapped);
        }

        AddressType::Enum getType() const {
            return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getType(); }, wrapped);
        }
        
        bool operator==(const AnyScript& other) const {
            return getType() == other.getType() && getScriptNum() == other.getScriptNum();
        }
        
        bool operator!=(const AnyScript& other) const {
            return !operator==(other);
        }
        
        std::string toString() const {
            return mpark::visit([&](auto &scriptAddress) { return scriptAddress.toString(); }, wrapped);
        }
        
        std::string toPrettyString() const {
            return mpark::visit([&](auto &scriptAddress) { return scriptAddress.toPrettyString(); }, wrapped);
        }
        
        void visitPointers(const std::function<void(const Address &)> &func) {
            mpark::visit([&](auto &scriptAddress) { scriptAddress.visitPointers(func); }, wrapped);
        }
        
        uint32_t firstTxIndex() {
            return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getFirstTxIndex(); }, wrapped);
        }
        
        ranges::optional<uint32_t> txRevealedIndex() {
            return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getTxRevealedIndex(); }, wrapped);
        }

        Transaction getFirstTransaction() const;
        ranges::optional<Transaction> getTransactionRevealed() const;

        bool hasBeenSpent() const {
            return mpark::visit([&](auto &scriptAddress) { return scriptAddress.hasBeenSpent(); }, wrapped);
        }
    
        EquivAddress getEquivAddresses(bool nestedEquivalent) const;
        
        auto getOutputPointers() {
            return mpark::visit([&](auto &scriptAddress) { return scriptAddress.getOutputPointers(); }, wrapped);
        }
        
        int64_t calculateBalance(BlockHeight height) const;
        ranges::any_view<Output> getOutputs() const;
        ranges::any_view<Input> getInputs() const;
        std::vector<Transaction> getTransactions() const;
        ranges::any_view<Transaction> getOutputTransactions() const;
        std::vector<Transaction> getInputTransactions() const;
        
        ScriptVariant wrapped;
    };
} // namespace blocksci

namespace std {
    template <>
    struct hash<blocksci::AnyScript> {
        typedef blocksci::AnyScript argument_type;
        typedef size_t  result_type;
        result_type operator()(const argument_type &b) const {
            return blocksci::hashAddress(b.getScriptNum(), b.getType());
        }
    };
}

#endif /* script_variant_hpp */
