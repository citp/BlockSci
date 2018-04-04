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
#include "scripthash_script.hpp"
#include "script_info.hpp"
#include "scripts_fwd.hpp"

#include <blocksci/address/address_info.hpp>
#include <blocksci/address/address_fwd.hpp>
#include <blocksci/chain/chain_fwd.hpp>

#include <range/v3/utility/optional.hpp>
#include <mpark/variant.hpp>

#include <functional>

namespace blocksci {
    class AnyScript {
    public:
        using ScriptVariant = to_variadic_t<to_address_tuple_t<ScriptAddress>, mpark::variant>;
        
        AnyScript(const Address &address, const DataAccess &access);
        AnyScript(uint32_t addressNum, AddressType::Enum type, const DataAccess &access);
        
        AddressType::Enum type() const;
        
        std::string toString() const;
        std::string toPrettyString() const;
        
        void visitPointers(const std::function<void(const Address &)> &func);
        
        uint32_t firstTxIndex();
        uint32_t txRevealedIndex();
        
        Transaction getFirstTransaction() const;
        ranges::optional<Transaction> getTransactionRevealed() const;
        
        ScriptVariant wrapped;
    };
} // namespace blocksci

#endif /* script_variant_hpp */
