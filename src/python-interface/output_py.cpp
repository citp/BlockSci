//
//  output_py.cpp
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

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

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
    .def_property_readonly("tx", func([](const Output &output) {
        return output.transaction();
    }), "The transaction that contains this input")
    .def_property_readonly("block", func([](const Output &output) {
        return output.block();
    }), "The block that contains this input")
    ;
}

template <typename Class, typename FuncApplication>
void addOutputRangeMethods(Class &cl, FuncApplication func) {
    using Range = typename Class::type;
    cl
    .def_property_readonly("unspent",  [=](Range &range) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsUnspent(r);
        });
    })
    .def("spent_before",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentBeforeHeight(r, height);
        });
    })
    .def("spent_after",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentAfterHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("spent_within",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentWithinRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("spent_outside",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentOutsideRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    })
    .def("with_type", [=](Range &range, AddressType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsOfType(std::forward<decltype(r)>(r), type);
        });
    })
    .def("with_type", [=](Range &range, ScriptType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsOfType(std::forward<decltype(r)>(r), type);
        });
    })
    ;
}


void init_output(py::module &m) {
    py::class_<OutputPointer>(m, "OutputPointer", "Class representing a pointer to an output")
    .def("__repr__", &OutputPointer::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("tx_index", &OutputPointer::txNum)
    .def_readonly("out_index", &OutputPointer::inoutNum)
    ;
    
    py::class_<Output> outputClass(m, "Output", "Class representing a transaction output");
    outputClass
    .def("__repr__", &Output::toString)
    .def(py::init<OutputPointer>())
    .def(py::self == py::self)
    .def(hash(py::self))
    ;
    
    addOutputMethods(outputClass, [](auto func) {
        return [=](Output &output) {
            return func(output);
        };
    });
    
    auto outputRangeClass = addRangeClass<ranges::any_view<Output>>(m, "AnyOutputRange");
    addOutputMethods(outputRangeClass, [](auto func) {
        return [=](ranges::any_view<Output> &range) {
            py::list list;
            RANGES_FOR(auto && output, range) {
                list.append(func(std::forward<decltype(output)>(output)));
            }
            return list;
        };
    });
    addOutputRangeMethods(outputRangeClass, [](ranges::any_view<Output> &range, auto func) {
        return func(range);
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
    addOutputRangeMethods(outputRangeClass2, [](ranges::any_view<Output, ranges::category::random_access | ranges::category::sized> &range, auto func) {
        return func(range);
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
    addOutputRangeMethods(nestedOutputRangeClass, [](ranges::any_view<ranges::any_view<Output>> &view, auto func) -> ranges::any_view<ranges::any_view<Output>> {
        return view | ranges::view::transform(func);
    });
}
