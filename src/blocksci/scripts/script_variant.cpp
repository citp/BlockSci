//
//  script_variant.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/14/17.
//

#include "script_variant.hpp"
#include "scripts.hpp"

#include <boost/variant.hpp>

namespace blocksci {
    
    template<ScriptType::Enum type>
    struct ScriptCreateFunctor {
        static AnyScript::ScriptVariant f(const ScriptAccess &access, const Script &script) {
            return ScriptAddress<type>(access, script.scriptNum);
        }
    };
    
    static constexpr auto scriptCreator = make_dynamic_table<ScriptType, ScriptCreateFunctor>();
    
    AnyScript::AnyScript(const Script &script, const ScriptAccess &access) :  wrapped(scriptCreator[static_cast<size_t>(script.type)](access, script)) {}
    
    AnyScript::AnyScript(const Address &address, const ScriptAccess &access) : AnyScript(Script(address.scriptNum, scriptType(address.type)), access) {}
    
    void AnyScript::visitPointers(const std::function<void(const Address &)> &func) {
        boost::apply_visitor([&](auto &scriptAddress) { scriptAddress.visitPointers(func); }, wrapped);
    }
    
    std::string AnyScript::toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const {
        return boost::apply_visitor([&](auto &scriptAddress) { return scriptAddress.toPrettyString(config, access); }, wrapped);
    }
}
