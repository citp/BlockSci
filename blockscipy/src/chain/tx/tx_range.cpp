//
//  tx_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "tx_py.hpp"
#include "ranges_py.hpp"
#include "caster_py.hpp"

namespace py = pybind11;
using namespace blocksci;

void addTxRangeMethods(RangeClasses<Transaction> &classes) {
    addAllRangeMethods(classes);
    addTxOptionalRangeMethods(classes);
}