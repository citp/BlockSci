//
//  input_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "input_py.hpp"
#include "optional_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

#include <range/v3/range_for.hpp>
#include <range/v3/view/transform.hpp>

#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

void init_input(py::module &m) {
    py::class_<Input> inputClass(m, "Input", "Class representing a transaction input");
    inputClass
    .def("__repr__", &Input::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &Input::getAccess)
    ;
    
    addInputMethods(inputClass, [](auto func) {
        return [=](Input &input) {
            return func(input);
        };
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    auto inputRangeClass = addRangeClass<ranges::any_view<Input>>(m, "AnyInputRange");
    addInputMethods(inputRangeClass, [](auto func) {
        return [=](ranges::any_view<Input> &view) {
            py::list list;
            RANGES_FOR(const auto &input, view) {
                list.append(func(input));
            }
            return list;
        };
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each input: " << docstring;
        return strdup(ss.str().c_str());
    });
    addInputRangeMethods(inputRangeClass, [](ranges::any_view<Input> &range, auto func) {
        return func(range);
    });
    
    auto inputRangeClass2 = addRangeClass<ranges::any_view<Input, ranges::category::random_access | ranges::category::sized>>(m, "InputRange");
    addInputMethods(inputRangeClass2, [](auto func) {
        return [=](ranges::any_view<Input, ranges::category::random_access | ranges::category::sized> &view) {
            py::list list;
            RANGES_FOR(const auto &input, view) {
                list.append(func(input));
            }
            return list;
        };
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each input: " << docstring;
        return strdup(ss.str().c_str());
    });
    addInputRangeMethods(inputRangeClass2, [](ranges::any_view<Input, ranges::category::random_access | ranges::category::sized> &view, auto func) {
        return func(view);
    });

    auto nestedInputRangeClass = addRangeClass<ranges::any_view<ranges::any_view<Input>>>(m, "NestedInputRange");
    addInputMethods(nestedInputRangeClass, [](auto func) {
        return [=](ranges::any_view<ranges::any_view<Input>> &view) {
            py::list list;
            RANGES_FOR(ranges::any_view<Input> inputRange, view) {
                py::list nestedList;
                RANGES_FOR(const auto &input, inputRange) {
                    nestedList.append(func(input));
                }
                list.append(nestedList);
            }
            return list;
        };
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each input: " << docstring;
        return strdup(ss.str().c_str());
    });
    addInputRangeMethods(nestedInputRangeClass, [](ranges::any_view<ranges::any_view<Input>> &view, auto func) -> ranges::any_view<ranges::any_view<Input>> {
        return ranges::view::transform(view, [=](auto && nestedView) {
            return func(std::forward<decltype(nestedView)>(nestedView));
        });
    });
}
