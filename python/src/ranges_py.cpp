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
        py::class_<ranges::any_view<ranges::optional<int64_t>>> cl(m, "IntOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<int64_t>, ranges::category::random_access>> cl(m, "IntOptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>>> cl(m, "DateOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>, ranges::category::random_access>> cl(m, "DateOptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<uint256>>> cl(m, "Hash256OptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<uint256>, ranges::category::random_access>> cl(m, "Hash256OptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<bool>>> cl(m, "BoolOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<bool>, ranges::category::random_access>> cl(m, "BoolOptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<AddressType::Enum>>> cl(m, "AddressTypeOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<AddressType::Enum>, ranges::category::random_access>> cl(m, "AddressTypeOptionalRange");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<py::bytes>>> cl(m, "BytesOptionalIterator");
        addRangeMethods(cl);
    }
    {
        py::class_<ranges::any_view<ranges::optional<py::bytes>, ranges::category::random_access>> cl(m, "BytesOptionalRange");
        addRangeMethods(cl);
    }
}
