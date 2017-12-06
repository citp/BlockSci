//
//  tx_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "optional_py.hpp"

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

#include <range/v3/iterator_range.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>

namespace py = pybind11;

using namespace blocksci;

template <typename Class, typename FuncApplication>
void addTransactionMethods(Class &cl, FuncApplication func) {
    cl
    .def_property_readonly("output_count", func([](const Transaction &tx) {
        return tx.outputCount();
    }), "The number of outputs this transaction has")
    .def_property_readonly("input_count", func([](const Transaction &tx) {
        return tx.inputCount();
    }), "The number of inputs this transaction has")
    .def_property_readonly("size_bytes", func([](const Transaction &tx) {
        return tx.sizeBytes();
    }), "The size of this transaction in bytes")
    .def_property_readonly("locktime", func([](const Transaction &tx) {
        return tx.locktime();
    }), "The locktime of this transasction")
    .def_property_readonly("block_height", func([](const Transaction &tx) {
        return tx.blockHeight;
    }), "The height of the block that this transaction was in")
    .def_property_readonly("block_time", func([](const Transaction &tx) {
        return tx.block().getTime();
    }))
    .def_property_readonly("block", func([](const Transaction &tx) {
        return tx.block();
    }), "The block that this transaction was in")
    .def_property_readonly("index", func([](const Transaction &tx) {
        return tx.block();
    }), "The internal index of this transaction")
    .def_property_readonly("hash", func([](const Transaction &tx) {
        return tx.getHash();
    }), "The 256-bit hash of this transaction")
    .def_property_readonly("input_value", func([](const Transaction &tx) {
        return totalInputValue(tx);
    }), "The sum of the value of all of the inputs")
    .def_property_readonly("output_value", func([](const Transaction &tx) {
        return totalOutputValue(tx);
    }), "The sum of the value of all of the outputs")
    .def_property_readonly("output_value", func([](const Transaction &tx) {
        return totalOutputValue(tx);
    }), "The sum of the value of all of the outputs")
    .def_property_readonly("fee", func([](const Transaction &tx) {
        return fee(tx);
    }), "The fee paid by this transaction")
    .def_property_readonly("fee_per_byte", func([](const Transaction &tx) {
        return feePerByte(tx);
    }), "The ratio of fee paid to size in bytes of this transaction")
    .def_property_readonly("op_return", func([](const Transaction &tx) {
        return getOpReturn(tx);
    }), "If this transaction included a null data script, return its output. Otherwise return None")
    .def_property_readonly("is_coinbase", func([](const Transaction &tx) {
        return tx.isCoinbase();
    }), "Return's true if this transaction is a Coinbase transaction")
    .def_property_readonly("change_output", func([](const Transaction &tx) {
        return heuristics::uniqueChangeByLegacyHeuristic(tx);
    }), "If the change address in this transaction can be determined via the fresh address criteria, return it. Otherwise return None.")
    ;
}

void init_tx(py::module &m) {
    
    py::class_<ranges::any_view<Transaction>> txRangeClass(m, "TxRange", "Class representing a list of transactions");
    addTransactionMethods(txRangeClass, [](auto func) {
        return [=](ranges::any_view<Transaction> &view) {
            py::list list;
            RANGES_FOR(auto && tx, view) {
                list.append(func(std::forward<decltype(tx)>(tx)));
            }
            return list;
        };
    });

    
    py::class_<Transaction> txClass(m, "Tx", "Class representing a transaction in a block");
    addTransactionMethods(txClass, [](auto func) {
        return [=](Transaction &tx) {
            return func(tx);
        };
    });
    
    txClass
    .def("__str__", &Transaction::getString)
    .def("__repr__", &Transaction::getString)
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
    .def_property_readonly("is_coinjoin", py::overload_cast<const Transaction &>(heuristics::isCoinjoin), "Uses basic structural features to quickly decide whether this transaction might be a JoinMarket coinjoin transaction")
    .def_property_readonly("is_script_deanon", py::overload_cast<const Transaction &>(heuristics::isDeanonTx), "Returns true if this transaction's change address is deanonymized by the script types involved")
    .def_property_readonly("is_change_over", py::overload_cast<const Transaction &>(heuristics::isChangeOverTx), "Returns true if this transaction contained all inputs of one address type and all outputs of a different type")
    .def_property_readonly("is_keyset_change", py::overload_cast<const Transaction &>(heuristics::containsKeysetChange), "Returns true if this transaction contains distinct addresses which share some of the same keys, indicating that the access control structure has changed")
    .def_property_readonly("is_definite_coinjoin", [](const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth) {
        py::gil_scoped_release release;
        return heuristics::isCoinjoinExtra(tx, minBaseFee, percentageFee, maxDepth);
    }, "This function uses subset matching in order to determine whether this transaction is a JoinMarket coinjoin. If maxDepth != 0, it limits the total number of possible subsets the algorithm will check.")
    .def_property_readonly("is_definite_coinjoin", [](const Transaction &tx, uint64_t minBaseFee, double percentageFee) {
        py::gil_scoped_release release;
        return heuristics::isCoinjoinExtra(tx, minBaseFee, percentageFee, 0);
    }, "This function uses subset matching in order to determine whether this transaction is a JoinMarket coinjoin.")
    ;
//
//    py::enum_<CoinJoinResult>(m, "CoinJoinResult")
//    .value("True", CoinJoinResult::True)
//    .value("False", CoinJoinResult::False)
//    .value("Timeout", CoinJoinResult::Timeout)
//    ;
}
