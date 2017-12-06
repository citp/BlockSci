//
//  input_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "optional_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>

#include <range/v3/range_for.hpp>

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
    .def_property_readonly("spent_tx_index", func([](const Input &input) {
        return input.spentTxIndex();
    }), "The index of the transaction that this input spent")
    .def_property_readonly("spent_tx", func([](const Input &input) {
        return input.getSpentTx();
    }), "The transaction that this input spent")
    .def_property_readonly("age", func([](const Input &input) {
        return input.age();
    }), "The number of blocks between the spent output and this input")
    ;
}

template <typename Class, typename FuncApplication>
void addOutputMethods(Class &cl, FuncApplication func) {
    cl
    .def_property_readonly("address", func([](const Output &output) {
        return output.getAddress();
    }), "This address linked to this output")
    .def_property_readonly("value", func([](const Output &output) {
        return output.getValue();
    }), "The value in base currency attached to this output")
    .def_property_readonly("address_type", func([](const Output &output) {
        return output.getType();
    }), "The address type of the output")
    .def_property_readonly("is_spent", func([](const Output &output) {
        return output.isSpent();
    }), "Returns whether this output has been spent")
    .def_property_readonly("spending_tx_index", func([](const Output &output) {
        return output.getSpendingTxIndex();
    }), "Returns the index of the tranasction which spent this output or 0 if it is unspent")
    .def_property_readonly("spending_tx", func([](const Output &output) {
        return output.getSpendingTx();
    }), "Returns the transaction that spent this output or None if it is unspent")
    ;
}

void init_inout(py::module &m) {
    py::class_<OutputPointer>(m, "OutputPointer", "Class representing a pointer to an output")
    .def("__repr__", &OutputPointer::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("tx_index", &OutputPointer::txNum)
    .def_readonly("out_index", &OutputPointer::inoutNum)
    ;

    py::class_<InputPointer>(m, "InputPointer", "Class representing a pointer to an input")
    .def("__repr__", &InputPointer::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("tx_index", &InputPointer::txNum)
    .def_readonly("int_index", &InputPointer::inoutNum)
    ;
    
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
    
    auto outputRangeClass = addRangeClass<ranges::any_view<Output>>(m, "AnyOutputRange");
    addOutputMethods(outputRangeClass, [](auto func) {
        return [=](ranges::any_view<Output> &view) {
            py::list list;
            RANGES_FOR(auto && output, view) {
                list.append(func(std::forward<decltype(output)>(output)));
            }
            return list;
        };
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

    auto outputRangeClass2 = addRangeClass<ranges::any_view<Output, ranges::category::random_access | ranges::category::sized>>(m, "OutputRange");
    addOutputMethods(outputRangeClass2, [](auto func) {
        return [=](ranges::any_view<Output, ranges::category::random_access | ranges::category::sized> &view) {
            py::list list;
            RANGES_FOR(auto && output, view) {
                list.append(func(std::forward<decltype(output)>(output)));
            }
            return list;
        };
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

    auto nestedOutputRangeClass = addRangeClass<ranges::any_view<ranges::any_view<Output>>>(m, "NestedOutputRange");
    addOutputMethods(nestedOutputRangeClass, [](auto func) {
        return [=](ranges::any_view<ranges::any_view<Output>> &view) {
            py::list list;
            RANGES_FOR(auto && outputRange, view) {
                py::list nestedList;
                RANGES_FOR(auto && output, outputRange) {
                    nestedList.append(func(std::forward<decltype(output)>(output)));
                }
                list.append(nestedList);
            }
            return list;
        };
    });

    py::class_<Output> outputClass(m, "Output", "Class representing a transaction output");
    outputClass
    .def("__repr__", &Output::toString)
    .def(py::init<OutputPointer>())
    .def(py::self == py::self)
    .def(hash(py::self))
    ;
    
    addOutputMethods(inputClass, [](auto func) {
        return [=](Output &output) {
            return func(output);
        };
    });
    

}
