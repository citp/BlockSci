//
//  type_converter.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "type_converter.hpp"

#include <blocksci/scripts/script_variant.hpp>

#include <pybind11/pybind11.h>

using namespace blocksci;

blocksci::AnyScript BasicTypeConverter::operator()(const blocksci::Address &val) {
    return val.getScript();
}
