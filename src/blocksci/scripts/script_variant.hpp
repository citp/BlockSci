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

#include <functional>

namespace blocksci {
    struct Address;
    
    using ScriptVariant = to_script_variant_t<ScriptAddress>;
    
    void visitPointers(const ScriptVariant &var, const std::function<void(const Address &)> &func);
    
    std::string toPrettyString(const ScriptVariant &var, const DataConfiguration &config, const ScriptAccess &access);
}

#endif /* script_variant_hpp */
