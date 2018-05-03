//
//  output_range_filters.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "tx_range_py.hpp"
#include "caster_py.hpp"
#include "range_filter_apply_py.hpp"

#include <blocksci/chain/algorithms.hpp>

namespace py = pybind11;
using namespace blocksci;

template <typename Range>
struct AddTxRangeMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(method_tag, "including_output_of_type", [](Range &range, AddressType::Enum type) {
            return range | ranges::view::filter([type](const Transaction &tx) {
                return includesOutputOfType(tx, type);
            });
        }, "Returns a range containing only transactions including an output of the given address type", pybind11::arg("address_type"));
    }
};

void applyRangeFiltersToTxRange(RangeClasses<Transaction> &classes) {
    applyRangeFiltersToRange<AddTxRangeMethods>(classes);
}
