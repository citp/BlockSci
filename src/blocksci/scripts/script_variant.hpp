//
//  script_variant.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/14/17.
//

#ifndef script_variant_hpp
#define script_variant_hpp

#include "script_info.hpp"
#include "scriptsfwd.hpp"
#include "scripts.hpp"

#include <boost/variant/variant.hpp>

#include <functional>

namespace blocksci {
    struct Address;
    
    class AnyScript {
    public:
        using ScriptVariant = to_script_variant_t<ScriptAddress>;
        
        AnyScript(const Address &address, const ScriptAccess &access);
        AnyScript(const Script &script, const ScriptAccess &access);
        
        std::string toString(const DataConfiguration &config) const;
        std::string toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const;
        
        void visitPointers(const std::function<void(const Address &)> &func);
        
        #ifndef BLOCKSCI_WITHOUT_SINGLETON
        AnyScript(const Address &address);
        AnyScript(const Script &script);
        std::string toString() const;
        std::string toPrettyString() const;
        #endif
        
        ScriptVariant wrapped;
    };
}

#endif /* script_variant_hpp */
