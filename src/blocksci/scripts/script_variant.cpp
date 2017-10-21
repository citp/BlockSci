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
    struct VisitPointersVisitor : public boost::static_visitor<void> {
        const std::function<void(const Address &)> &func;
        
        VisitPointersVisitor(const std::function<void(const Address &)> & func_) : func(func_) {}
        template <ScriptType::Enum type>
        void operator()(const ScriptAddress<type> &data) const
        {
            data.visitPointers(func);
        }
    };
    
    void visitPointers(const ScriptVariant &var, const std::function<void(const Address &)> &func) {
        boost::apply_visitor(VisitPointersVisitor(func), var);
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
    
    std::string toPrettyString(const ScriptVariant &var, const DataConfiguration &config, const ScriptAccess &access) {
        return boost::apply_visitor(PrettyPrintVisitor(config, access), var);
    }
}
