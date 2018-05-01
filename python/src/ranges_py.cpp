//
//  ranges_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#include "ranges_py.hpp"
#include "caster_py.hpp"

#include <pybind11/chrono.h>

namespace py = pybind11;
using namespace blocksci;

void init_ranges(py::module &m) {
    {
        py::class_<ranges::any_view<ranges::optional<int64_t>>> cl(m, "OptionalIntIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<int64_t>, ranges::category::random_access>> cl(m, "OptionalIntRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>>> cl(m, "OptionalDateIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>, ranges::category::random_access>> cl(m, "OptionalDateRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<uint256>>> cl(m, "OptionalHash256Iterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<uint256>, ranges::category::random_access>> cl(m, "OptionalHash256Range");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<bool>>> cl(m, "OptionalBoolIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<bool>, ranges::category::random_access>> cl(m, "OptionalBoolRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<AddressType::Enum>>> cl(m, "OptionalAddressTypeIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<AddressType::Enum>, ranges::category::random_access>> cl(m, "OptionalAddressTypeRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<py::bytes>>> cl(m, "OptionalBytesIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<py::bytes>, ranges::category::random_access>> cl(m, "OptionalBytesRange");
        addRangeMethods(cl);
    }
}
