//
//  script_variant.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/14/17.
//

#ifndef script_variant_hpp
#define script_variant_hpp

#include "script_info.hpp"
#include "scripts_fwd.hpp"
#include "pubkey_script.hpp"
#include "scripthash_script.hpp"
#include "multisig_script.hpp"
#include "nulldata_script.hpp"
#include "nonstandard_script.hpp"

#include <mpark/variant.hpp>

#include <functional>

namespace blocksci {
    struct Address;
    
    class AnyScript {
    public:
        using ScriptVariant = to_variadic_t<to_script_tuple_t<ScriptAddress>, mpark::variant>;
        
        AnyScript(const Address &address, const ScriptAccess &access);
        AnyScript(const Script &script, const ScriptAccess &access);
        
        std::string toString() const;
        std::string toPrettyString() const;
        
        void visitPointers(const std::function<void(const Address &)> &func);
        
        uint32_t firstTxIndex();
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        AnyScript(const Address &address);
        AnyScript(const Script &script);
        #endif
        
        ScriptVariant wrapped;
    };
}

#endif /* script_variant_hpp */
