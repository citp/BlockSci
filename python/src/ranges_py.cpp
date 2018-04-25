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
    addRangeClass<ranges::any_view<ranges::optional<int64_t>>>(m, "AnyOptionalIntRange");
    addRangeClass<ranges::any_view<ranges::optional<int64_t>, ranges::category::random_access>>(m, "OptionalIntRange");
    addRangeClass<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>>>(m, "AnyOptionalDateRange");
    addRangeClass<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>, ranges::category::random_access>>(m, "OptionalDateRange");
    addRangeClass<ranges::any_view<ranges::optional<uint256>>>(m, "AnyOptionalHash256Range");
    addRangeClass<ranges::any_view<ranges::optional<uint256>, ranges::category::random_access>>(m, "OptionalHash256Range");
    addRangeClass<ranges::any_view<ranges::optional<bool>>>(m, "AnyOptionalBoolRange");
    addRangeClass<ranges::any_view<ranges::optional<bool>, ranges::category::random_access>>(m, "OptionalBoolRange");
    addRangeClass<ranges::any_view<ranges::optional<AddressType::Enum>>>(m, "AnyOptionalAddressTypeRange");
    addRangeClass<ranges::any_view<ranges::optional<AddressType::Enum>, ranges::category::random_access>>(m, "OptionalAddressTypeRange");
    addRangeClass<ranges::any_view<ranges::optional<py::bytes>>>(m, "AnyOptionalBytesRange");
    addRangeClass<ranges::any_view<ranges::optional<py::bytes>, ranges::category::random_access>>(m, "OptionalBytesRange");
}
