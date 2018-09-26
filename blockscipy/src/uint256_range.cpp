//
//  uint256_range.cpp
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

void init_uint256_range(py::module &m) {
    constexpr auto rangeCat = ranges::category::random_access | ranges::category::sized;
    {
        py::class_<ranges::any_view<ranges::optional<uint256>>> cl(m, "UInt256OptionalIterator");
        addOptionalRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<uint256>, rangeCat>> cl(m, "UInt256OptionalRange");
        addOptionalRangeMethods(cl);
    }
}
