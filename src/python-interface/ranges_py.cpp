//
//  ranges_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#include "ranges_py.hpp"

#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace blocksci;

void init_ranges(py::module &m) {
	addRangeClass<ranges::any_view<Block>>(m, "AnyBlockRange");
    addRangeClass<ranges::any_view<Transaction>>(m, "AnyTransactionRange");
    addRangeClass<ranges::any_view<Output>>(m, "AnyOutputRange");
    addRangeClass<ranges::any_view<Input>>(m, "AnyInputRange");
	addRangeClass<ranges::any_view<Output, ranges::category::random_access | ranges::category::sized>>(m, "OutputRange");
    addRangeClass<ranges::any_view<Input, ranges::category::random_access | ranges::category::sized>>(m, "InputRange");
}
