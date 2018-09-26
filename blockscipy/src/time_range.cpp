//
//  time_range.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/chrono.h>

namespace py = pybind11;
using namespace blocksci;

void init_time_range(py::module &m) {
    constexpr auto rangeCat = ranges::category::random_access | ranges::category::sized;
    {
        py::class_<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>>> cl(m, "DateOptionalIterator");
        addOptionalRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>, rangeCat>> cl(m, "DateOptionalRange");
        addOptionalRangeMethods(cl);
    }
}
