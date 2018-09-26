//
//  input_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "input_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/input.hpp>
#include <blocksci/chain/input_range.hpp>
#include <blocksci/chain/output_range.hpp>

namespace py = pybind11;
using namespace blocksci;

void addInputRangeMethods(RangeClasses<Input> &classes) {
    addAllRangeMethods(classes);
    addInputOptionalRangeMethods(classes);
}