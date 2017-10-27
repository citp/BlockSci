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
    
    AnyScript::AnyScript(const Script &script, const ScriptAccess &access) :  script(scriptCreator[static_cast<size_t>(script.type)](access, script)) {}
    
    AnyScript::AnyScript(const Address &address, const ScriptAccess &access) : AnyScript(Script(address.scriptNum, scriptType(address.type)), access) {}
    
    struct VisitPointersVisitor : public boost::static_visitor<void> {
        const std::function<void(const Address &)> &func;
        
        VisitPointersVisitor(const std::function<void(const Address &)> & func_) : func(func_) {}
        template <ScriptType::Enum type>
        void operator()(const ScriptAddress<type> &data) const
        {
            data.visitPointers(func);
        }
    };
    
    void AnyScript::visitPointers(const std::function<void(const Address &)> &func) {
        boost::apply_visitor(VisitPointersVisitor(func), script);
    }
    
    struct PrettyPrintVisitor : public boost::static_visitor<std::string> {
        const DataConfiguration &config;
        const ScriptAccess &access;
        
        PrettyPrintVisitor(const DataConfiguration &config_, const ScriptAccess &access_) : config(config_), access(access_) {}
        template <ScriptType::Enum type>
        std::string operator()(const ScriptAddress<type> &data) const
        {
            return data.toPrettyString(config, access);
        }
    };
    
    std::string AnyScript::toPrettyString(const DataConfiguration &config, const ScriptAccess &access) const {
        return boost::apply_visitor(PrettyPrintVisitor(config, access), script);
    }
}
