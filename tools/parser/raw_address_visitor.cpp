//
//  raw_address_visitor.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "raw_address_visitor.hpp"

#include <blocksci/core/address_type_meta.hpp>
#include <blocksci/core/meta.hpp>
#include <blocksci/core/script_data.hpp>

#include <internal/address_info.hpp>
#include <internal/script_access.hpp>

#include <mpark/variant.hpp>

using namespace blocksci;

template <AddressType::Enum addressType>
using ScriptDataPointer = const typename ScriptInfo<dedupType(addressType)>::outputType*;

template <AddressType::Enum type>
struct ScriptWrapper {
    ScriptDataPointer<type> data;
};
using ScriptDataVariant = to_variadic_t<to_address_tuple_t<ScriptWrapper>, mpark::variant>;

template<AddressType::Enum type>
struct ScriptDataCreateFunctor {
    static ScriptDataVariant f(uint32_t scriptNum, const ScriptAccess &access) {
        auto &file = access.getFile<dedupType(type)>();
        return ScriptWrapper<type>{file.getDataAtIndex(scriptNum - 1)};
    }
};

static constexpr auto scriptDataCreator = make_dynamic_table<AddressType, ScriptDataCreateFunctor>();

class AnyScriptData {
public:
    AnyScriptData(const RawAddress &address, const ScriptAccess &access) : wrapped(scriptDataCreator.at(static_cast<size_t>(address.type))(address.scriptNum, access)) {}
    
    void visitPointers(const std::function<void(const RawAddress &)> &func) {
        mpark::visit([&](auto &scriptAddress) { scriptAddress.data->visitPointers(func); }, wrapped);
    }
    
    ScriptDataVariant wrapped;
};

void visit(const RawAddress &address, const std::function<bool(const RawAddress &)> &visitFunc, const ScriptAccess &scripts) {
    if (visitFunc(address)) {
        std::function<void(const RawAddress &)> nestedVisitor = [&](const RawAddress &nestedAddress) {
            visit(nestedAddress, visitFunc, scripts);
        };
        AnyScriptData script{address, scripts};
        script.visitPointers(nestedVisitor);
    }
}



