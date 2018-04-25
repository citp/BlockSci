//
//  self_apply_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "self_apply_py.hpp"

#include <blocksci/scripts/script_variant.hpp>

using namespace blocksci;

blocksci::AnyScript SelfTypeConverter<blocksci::Address>::operator()(const blocksci::Address &val) {
    return val.getScript();
}
