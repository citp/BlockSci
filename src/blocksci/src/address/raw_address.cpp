//
//  raw_address.cpp
//  bitcoinapi
//
//  Created by Harry Kalodner on 4/10/18.
//

#include <blocksci/address/raw_address.hpp>
#include <blocksci/scripts/script_variant.hpp>

namespace blocksci {
    void visit(const RawAddress &address, const std::function<bool(const RawAddress &)> &visitFunc, const ScriptAccess &scripts) {
        if (visitFunc(address)) {
            std::function<void(const RawAddress &)> nestedVisitor = [&](const RawAddress &nestedAddress) {
                visit(nestedAddress, visitFunc, scripts);
            };
            AnyScriptData script{address, scripts};
            script.visitPointers(nestedVisitor);
        }
    }
} // namespace blocksci
