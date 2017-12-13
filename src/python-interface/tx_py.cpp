//
//  tx_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "optional_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/heuristics/tx_identification.hpp>
#include <blocksci/heuristics/change_address.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/chrono.h>

#include <range/v3/iterator_range.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>

namespace py = pybind11;

using namespace blocksci;

template <typename Class, typename FuncApplication, typename FuncDoc>
void addTransactionMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    cl
    .def_property_readonly("output_count", func([](const Transaction &tx) {
        return tx.outputCount();
    }), func2("The number of outputs this transaction has"))
    .def_property_readonly("input_count", func([](const Transaction &tx) {
        return tx.inputCount();
    }), func2("The number of inputs this transaction has"))
    .def_property_readonly("size_bytes", func([](const Transaction &tx) {
        return tx.sizeBytes();
    }), func2("The size of this transaction in bytes"))
    .def_property_readonly("locktime", func([](const Transaction &tx) {
        return tx.locktime();
    }), func2("The locktime of this transasction"))
    .def_property_readonly("block_height", func([](const Transaction &tx) {
        return tx.blockHeight;
    }), func2("The height of the block that this transaction was in"))
    .def_property_readonly("block_time", func([](const Transaction &tx) {
        return tx.block().getTime();
    }), func2("The time that the block containing this transaction arrived"))
    .def_property_readonly("block", func([](const Transaction &tx) {
        return tx.block();
    }), func2("The block that this transaction was in"))
    .def_property_readonly("index", func([](const Transaction &tx) {
        return tx.block();
    }), func2("The internal index of this transaction"))
    .def_property_readonly("hash", func([](const Transaction &tx) {
        return tx.getHash();
    }), func2("The 256-bit hash of this transaction"))
    .def_property_readonly("input_value", func([](const Transaction &tx) {
        return totalInputValue(tx);
    }), func2("The sum of the value of all of the inputs"))
    .def_property_readonly("output_value", func([](const Transaction &tx) {
        return totalOutputValue(tx);
    }), func2("The sum of the value of all of the outputs"))
    .def_property_readonly("output_value", func([](const Transaction &tx) {
        return totalOutputValue(tx);
    }), func2("The sum of the value of all of the outputs"))
    .def_property_readonly("fee", func([](const Transaction &tx) {
        return fee(tx);
    }), func2("The fee paid by this transaction"))
    .def_property_readonly("fee_per_byte", func([](const Transaction &tx) {
        return feePerByte(tx);
    }), func2("The ratio of fee paid to size in bytes of this transaction"))
    .def_property_readonly("op_return", func([](const Transaction &tx) {
        return getOpReturn(tx);
    }), func2("If this transaction included a null data script, return its output. Otherwise return None"))
    .def_property_readonly("is_coinbase", func([](const Transaction &tx) {
        return tx.isCoinbase();
    }), func2("Return's true if this transaction is a Coinbase transaction"))
    .def_property_readonly("change_output", func([](const Transaction &tx) {
        return heuristics::uniqueChangeByLegacyHeuristic(tx);
    }), func2("If the change address in this transaction can be determined via the fresh address criteria, return it. Otherwise return None."))
    ;
}

void init_tx(py::module &m) {
    
    auto txRangeClass = addRangeClass<ranges::any_view<Transaction>>(m, "AnyTxRange");
    addTransactionMethods(txRangeClass, [](auto func) {
        return [=](ranges::any_view<Transaction> &view) {
            py::list list;
            RANGES_FOR(auto && tx, view) {
                list.append(func(std::forward<decltype(tx)>(tx)));
            }
            return list;
        };
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each transaction: " << docstring;
        return strdup(ss.str().c_str());
    });

    txRangeClass
    .def_property_readonly("inputs", [](ranges::any_view<Transaction> &txes) -> ranges::any_view<ranges::any_view<Input>>  {
        return txes | ranges::view::transform([](const Transaction &tx) -> ranges::any_view<Input> {
            return tx.inputs();
        });
    }, "A list of lists of the inputs of each transaction")
    .def_property_readonly("outputs", [](ranges::any_view<Transaction> &txes) -> ranges::any_view<ranges::any_view<Output>>  {
        return txes | ranges::view::transform([](const Transaction &tx) -> ranges::any_view<Output> {
            return tx.outputs();
        });
    }, "A list of lists of the outputs of each transaction")
    ;
    
    
    py::class_<Transaction> txClass(m, "Tx", "Class representing a transaction in a block");
    addTransactionMethods(txClass, [](auto func) {
        return [=](Transaction &tx) {
            return func(tx);
        };
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    txClass
    .def("__str__", &Transaction::toString)
    .def("__repr__", &Transaction::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def(py::init<uint32_t>(), R"docstring(
         This functions gets the transaction with given index.
         
         :param int index: The index of the transation.
         :returns: Tx
         )docstring")
    .def(py::init<std::string>(), R"docstring(
         This functions gets the transaction with given hash.
         
         :param string index: The hash of the transation.
         :returns: Tx
         )docstring")
    .def_property_readonly("ins", [](const Transaction &tx) -> ranges::any_view<Input, ranges::get_categories<decltype(tx.inputs())>()>  {
        return tx.inputs();
    }, "A list of the inputs of the transaction") // same as below
    .def_property_readonly("inputs", [](const Transaction &tx) -> ranges::any_view<Input, ranges::get_categories<decltype(tx.inputs())>()>  {
        return tx.inputs();
    }, "A list of the inputs of the transaction") // same as above
    .def_property_readonly("outs", [](const Transaction &tx) -> ranges::any_view<Output, ranges::get_categories<decltype(tx.outputs())>()>  {
        return tx.outputs();
    }, "A list of the outputs of the transaction") // same as below
    .def_property_readonly("outputs", [](const Transaction &tx) -> ranges::any_view<Output, ranges::get_categories<decltype(tx.outputs())>()>  {
        return tx.outputs();
    }, "A list of the outputs of the transaction") // same as above
    ;
}
