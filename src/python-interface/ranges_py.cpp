//
//  ranges_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#include "ranges_py.hpp"
#include "caster_py.hpp"

namespace py = pybind11;
using namespace blocksci;

void init_ranges(py::module &m) {    
    addOptionalRangeClass<ranges::any_view<ranges::optional<int64_t>>>(m, "AnyOptionalIntRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<int64_t>, ranges::category::random_access>>(m, "OptionalIntRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>>>(m, "AnyOptionalDateRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<std::chrono::system_clock::time_point>, ranges::category::random_access>>(m, "OptionalDateRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<uint256>>>(m, "AnyOptionalHash256Range");
    addOptionalRangeClass<ranges::any_view<ranges::optional<uint256>, ranges::category::random_access>>(m, "OptionalHash256Range");
    addOptionalRangeClass<ranges::any_view<ranges::optional<bool>>>(m, "AnyOptionalBoolRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<bool>, ranges::category::random_access>>(m, "OptionalBoolRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<AddressType::Enum>>>(m, "AnyOptionalAddressTypeRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<AddressType::Enum>, ranges::category::random_access>>(m, "OptionalAddressTypeRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<py::bytes>>>(m, "AnyOptionalBytesRange");
    addOptionalRangeClass<ranges::any_view<ranges::optional<py::bytes>, ranges::category::random_access>>(m, "OptionalBytesRange");
}
