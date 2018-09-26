//
//  int_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/cluster/cluster.hpp>

namespace py = pybind11;
using namespace blocksci;

void init_int_range(py::module &m) {
    constexpr auto rangeCat = ranges::category::random_access | ranges::category::sized;
    {
        py::class_<ranges::any_view<ranges::optional<int64_t>>> cl(m, "IntOptionalIterator");
        addOptionalRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<int64_t>, rangeCat>> cl(m, "IntOptionalRange");
        addOptionalRangeMethods(cl);
    }
}
