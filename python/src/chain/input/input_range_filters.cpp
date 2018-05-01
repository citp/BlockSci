//
//  input_range_filters.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "input_range_py.hpp"
#include "caster_py.hpp"
#include "range_filter_apply_py.hpp"

#include <blocksci/chain/algorithms.hpp>

namespace py = pybind11;
using namespace blocksci;

template <typename Range>
struct AddInputRangeMethods {
    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        py::options options;
        options.disable_function_signatures();
        cl
        .def("sent_before_height",  func([](Range &range, BlockHeight height) {
            return inputsCreatedBeforeHeight(range, height);
        }), py::arg("height"), 
        R"docstring(sent_before_height(height: int) -> InputIterator
        Filter the inputs to include only inputs which spent an output created before the given height
        
        Args:
            height (int): The height above which inputs are excluded.

        Returns:
            InputIterator: A iterator for the filtered range

        )docstring")
        .def("sent_after_height",  func([](Range &range, BlockHeight height) {
            return inputsCreatedAfterHeight(range, height);
        }), py::arg("height"),
        R"docstring(sent_after_height(height: int) -> InputIterator
        Filter the inputs to include only inputs which spent an output created after the given height

        Args:
            height (int): The height above which inputs are excluded.

        Returns:
            InputIterator: A iterator for the filtered range

        )docstring")
        .def("with_age_less_than",  func([](Range &range, BlockHeight height) {
            return inputsCreatedWithinRelativeHeight(range, height);
        }), py::arg("age"),
        R"docstring(with_age_less_than(age: int) -> InputIterator
        Filter the inputs to include only inputs with age less than the given value

        Args:
            age (int): The maximum age of the filtered inputs

        Returns:
            InputIterator: A iterator for the filtered range

        )docstring")
        .def("with_age_greater_than",  func([](Range &range, BlockHeight height) {
            return inputsCreatedOutsideRelativeHeight(range, height);
        }), py::arg("age"),
        R"docstring(with_age_greater_than(age: int) -> InputIterator
        Filter the inputs to include only inputs with age more than the given value

        Args:
            age (int): The minimum age of the filtered inputs

        Returns:
            InputIterator: A iterator for the filtered range
        
        )docstring")
        .def("with_address_type", func([](Range &range, AddressType::Enum type) {
            return inputsOfType(range, type);
        }), py::arg("type"), 
        R"docstring(with_address_type(type: address_type) -> InputIterator
        Filter the inputs to include only inputs that came from an address with the given type

        Filter the inputs to include only inputs with age more than the given value

        Args:
            type (address_type): The address type of the filtered inputs 

        Returns:
            InputIterator: A iterator for the filtered range
        
        )docstring")
        ;
    }
};

void applyRangeFiltersToInputRange(RangeClasses<Input> &classes) {
    applyRangeFiltersToRange<AddInputRangeMethods>(classes);
}
