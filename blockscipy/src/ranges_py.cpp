//
//  ranges_py.cpp
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

void init_ranges(py::module &m) {
    constexpr auto rangeCat = ranges::category::random_access | ranges::category::sized;
    {
        py::class_<ranges::any_view<ranges::optional<int64_t>>> cl(m, "IntOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<int64_t>, rangeCat>> cl(m, "IntOptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>>> cl(m, "DateOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>, rangeCat>> cl(m, "DateOptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<uint256>>> cl(m, "UInt256OptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<uint256>, rangeCat>> cl(m, "UInt256OptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<bool>>> cl(m, "BoolOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<bool>, rangeCat>> cl(m, "BoolOptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<AddressType::Enum>>> cl(m, "AddressTypeOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<AddressType::Enum>, rangeCat>> cl(m, "AddressTypeOptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<py::bytes>>> cl(m, "BytesOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<py::bytes>, rangeCat>> cl(m, "BytesOptionalRange");
        addRangeMethods(cl);
    }
}
