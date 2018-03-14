//
//  output_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 12/7/17.
//

#include "optional_py.hpp"
#include "ranges_py.hpp"
#include "variant_py.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/range_for.hpp>
#include <range/v3/view/transform.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

template <typename Class, typename FuncApplication, typename FuncDoc>
void addOutputMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    cl
    .def_property_readonly("address", func([](const Output &output) {
        return output.getAddress().getScript().wrapped;
    }), func2("This address linked to this output"))
    .def_property_readonly("value", func([](const Output &output) {
        return output.getValue();
    }), func2("The value in base currency attached to this output"))
    .def_property_readonly("address_type", func([](const Output &output) {
        return output.getType();
    }), func2("The address type of the output"))
    .def_property_readonly("is_spent", func([](const Output &output) {
        return output.isSpent();
    }), func2("Returns whether this output has been spent"))
    .def_property_readonly("spending_tx_index", func([](const Output &output) {
        return output.getSpendingTxIndex();
    }), func2("Returns the index of the tranasction which spent this output or 0 if it is unspent"))
    .def_property_readonly("spending_tx", func([](const Output &output) {
        return output.getSpendingTx();
    }), func2("Returns the transaction that spent this output or None if it is unspent"))
    .def_property_readonly("tx", func([](const Output &output) {
        return output.transaction();
    }), func2("The transaction that contains this input"))
    .def_property_readonly("block", func([](const Output &output) {
        return output.block();
    }), func2("The block that contains this input"))
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
    }, "Returns a range including the subset of outputs which were never spent")
    .def("spent_before",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentBeforeHeight(r, height);
        });
    }, "Returns a range including the subset of outputs which were spent before the given height")
    .def("spent_after",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentAfterHeight(std::forward<decltype(r)>(r), height);
        });
    }, "Returns a range including the subset of outputs which were spent after the given height")
    .def("spent_within",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentWithinRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    }, "Returns a range including the subset of outputs which were spent within the given number of blocks")
    .def("spent_outside",  [=](Range &range, blocksci::BlockHeight height) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsSpentOutsideRelativeHeight(std::forward<decltype(r)>(r), height);
        });
    }, "Returns a range including the subset of outputs which were spent later than the given number of blocks")
    .def("with_type", [=](Range &range, AddressType::Enum type) {
        return func(range, [=](auto && r) -> ranges::any_view<blocksci::Output> {
            return outputsOfType(std::forward<decltype(r)>(r), type);
        });
    }, "Returns a range including the subset of outputs which were sent to the given address type")
    ;
}


void init_output(py::module &m) {
    py::class_<Output> outputClass(m, "Output", "Class representing a transaction output");
    outputClass
    .def("__repr__", &Output::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    ;
    
    addOutputMethods(outputClass, [](auto func) {
        return [=](Output &output) {
            return func(output);
        };
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    auto outputRangeClass = addRangeClass<ranges::any_view<Output>>(m, "AnyOutputRange");
    addOutputMethods(outputRangeClass, [](auto func) {
        return [=](ranges::any_view<Output> &range) {
            py::list list;
            RANGES_FOR(const auto &output, range) {
                list.append(func(output));
            }
            return list;
        };
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each output: " << docstring;
        return strdup(ss.str().c_str());
    });
    addOutputRangeMethods(outputRangeClass, [](auto &&range, auto func) {
        return func(range);
    });
    
    auto outputRangeClass2 = addRangeClass<ranges::any_view<Output, ranges::category::random_access | ranges::category::sized>>(m, "OutputRange");
    addOutputMethods(outputRangeClass2, [](auto func) {
        return [=](ranges::any_view<Output, ranges::category::random_access | ranges::category::sized> &view) {
            py::list list;
            RANGES_FOR(const auto &output, view) {
                list.append(func(output));
            }
            return list;
        };
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each output: " << docstring;
        return strdup(ss.str().c_str());
    });
    addOutputRangeMethods(outputRangeClass2, [](ranges::any_view<Output, ranges::category::random_access | ranges::category::sized> &range, auto func) {
        return func(range);
    });
    
    
    auto nestedOutputRangeClass = addRangeClass<ranges::any_view<ranges::any_view<Output>>>(m, "NestedOutputRange");
    addOutputMethods(nestedOutputRangeClass, [](auto func) {
        return [=](ranges::any_view<ranges::any_view<Output>> &view) {
            py::list list;
            RANGES_FOR(ranges::any_view<Output> outputRange, view) {
                py::list nestedList;
                RANGES_FOR(const auto &output, outputRange) {
                    nestedList.append(func(output));
                }
                list.append(nestedList);
            }
            return list;
        };
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each output: " << docstring;
        return strdup(ss.str().c_str());
    });
    addOutputRangeMethods(nestedOutputRangeClass, [](ranges::any_view<ranges::any_view<Output>> &view, auto func) -> ranges::any_view<ranges::any_view<Output>> {
        return view | ranges::view::transform(func);
    });
}
