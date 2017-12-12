//
//  input_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "optional_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>

#include <range/v3/range_for.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/any_view.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

template <typename Class, typename FuncApplication>
void addInputMethods(Class &cl, FuncApplication func) {
    cl
    .def_property_readonly("address", func([](const Input &input) {
        return input.getAddress();
    }), "This address linked to this input")
    .def_property_readonly("value", func([](const Input &input) {
        return input.getValue();
    }), "The value in base currency attached to this inout")
    .def_property_readonly("address_type", func([](const Input &input) {
        return input.getType();
    }), "The address type of the input")
    .def_property_readonly("sequence_num", func([](const Input &input) {
        return input.sequenceNumber();
    }), "The sequence number of the input")
    .def_property_readonly("spent_tx_index", func([](const Input &input) {
        return input.spentTxIndex();
    }), "The index of the transaction that this input spent")
    .def_property_readonly("spent_tx", func([](const Input &input) {
        return input.getSpentTx();
    }), "The transaction that this input spent")
    .def_property_readonly("age", func([](const Input &input) {
        return input.age();
    }), "The number of blocks between the spent output and this input")
    .def_property_readonly("tx", func([](const Input &input) {
        return input.transaction();
    }), "The transaction that contains this input")
    .def_property_readonly("block", func([](const Input &input) {
        return input.block();
    }), "The block that contains this input")
    ;
}

template <typename Class, typename FuncApplication>
void addInputRangeMethods(Class &cl, FuncApplication func) {
    using Range = typename Class::type;
    cl
    .def("sent_before",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedBeforeHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("sent_after",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedAfterHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("sent_within",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedWithinRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("sent_outside",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsCreatedOutsideRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("with_type", [=](Range &range, AddressType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsOfType(std::forward<decltype(r)>(r), type);
        });
    })
    .def("with_type", [=](Range &range, ScriptType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Input> {
            return inputsOfType(std::forward<decltype(r)>(r), type);
        });
    })
    ;
}


void init_input(py::module &m) {
    py::class_<InputPointer>(m, "InputPointer", "Class representing a pointer to an input")
    .def("__repr__", &InputPointer::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("tx_index", &InputPointer::txNum)
    .def_readonly("int_index", &InputPointer::inoutNum)
    ;
    
    py::class_<Input> inputClass(m, "Input", "Class representing a transaction input");
    inputClass
    .def("__repr__", &Input::toString)
    .def(py::init<InputPointer>())
    .def(py::self == py::self)
    .def(hash(py::self))
    ;
    
    addInputMethods(inputClass, [](auto func) {
        return [=](Input &input) {
            return func(input);
        };
    });
    
    auto inputRangeClass = addRangeClass<ranges::any_view<Input>>(m, "AnyInputRange");
    addInputMethods(inputRangeClass, [](auto func) {
        return [=](ranges::any_view<Input> &view) {
            py::list list;
            RANGES_FOR(auto && input, view) {
                list.append(func(std::forward<decltype(input)>(input)));
            }
            return list;
        };
    });
    addInputRangeMethods(inputRangeClass, [](ranges::any_view<Input> &range, auto func) {
        return func(range);
    });
    
    auto inputRangeClass2 = addRangeClass<ranges::any_view<Input, ranges::category::random_access | ranges::category::sized>>(m, "InputRange");
    addInputMethods(inputRangeClass2, [](auto func) {
        return [=](ranges::any_view<Input, ranges::category::random_access | ranges::category::sized> &view) {
            py::list list;
            RANGES_FOR(auto && input, view) {
                list.append(func(std::forward<decltype(input)>(input)));
            }
            return list;
        };
    });
    addInputRangeMethods(inputRangeClass2, [](ranges::any_view<Input, ranges::category::random_access | ranges::category::sized> &view, auto func) {
        return func(view);
    });

    auto nestedInputRangeClass = addRangeClass<ranges::any_view<ranges::any_view<Input>>>(m, "NestedInputRange");
    addInputMethods(nestedInputRangeClass, [](auto func) {
        return [=](ranges::any_view<ranges::any_view<Input>> &view) {
            py::list list;
            RANGES_FOR(auto && inputRange, view) {
                py::list nestedList;
                RANGES_FOR(auto && input, inputRange) {
                    nestedList.append(func(std::forward<decltype(input)>(input)));
                }
                list.append(nestedList);
            }
            return list;
        };
    });
    addInputRangeMethods(nestedInputRangeClass, [](ranges::any_view<ranges::any_view<Input>> &view, auto func) -> ranges::any_view<ranges::any_view<Input>> {
        return ranges::view::transform(view, [=](auto && nestedView) {
            return func(std::forward<decltype(nestedView)>(nestedView));
        });
    });
}
