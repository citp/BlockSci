//
//  raw_address_visitor.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef raw_address_visitor_h
#define raw_address_visitor_h

#include <blocksci/core/core_fwd.hpp>

#include <functional>

namespace blocksci {
    class ScriptAccess;
}

void visit(const blocksci::RawAddress &address, const std::function<bool(const blocksci::RawAddress &)> &visitFunc, const blocksci::ScriptAccess &scripts);

#endif /* raw_address_visitor_h */
