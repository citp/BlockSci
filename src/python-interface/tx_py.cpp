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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <range/v3/iterator_range.hpp>
#include <range/v3/view/any_view.hpp>

namespace py = pybind11;

using namespace blocksci;

void init_tx(py::module &m) {
    py::class_<Transaction>(m, "Tx", "Class representing a transaction in a block")
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
    .def_property_readonly("num_outs", &Transaction::outputCount, "The number of outputs this transaction has")
    .def_property_readonly("num_ins", &Transaction::inputCount, "The number of inputs this transaction has")
    .def_property_readonly("size_bytes", &Transaction::sizeBytes, "The size of this transaction in bytes")
    .def_property_readonly("locktime", &Transaction::locktime, "The locktime of this transasction")
    .def_readonly("block_height", &Transaction::blockHeight, "The height of the block that this transaction was in")
    .def_property_readonly("block_time", [](const Transaction &tx) {
        return tx.block().getTime();
    })
    .def_property_readonly("block", py::overload_cast<>(&Transaction::block, py::const_), "The block that this transaction was in")
    .def_readonly("index", &Transaction::txNum, "The internal index of this transaction")
    .def_property_readonly("ins", [](const Transaction &tx) -> ranges::any_view<Input, ranges::get_categories<decltype(tx.inputs())>()>  {
        return tx.inputs();
    }, "A list of the inputs of the transaction")
    .def_property_readonly("outs", [](const Transaction &tx) -> ranges::any_view<Output, ranges::get_categories<decltype(tx.outputs())>()>  {
        return tx.outputs();
    }, "A list of the outputs of the transaction")
    .def_property_readonly("hash", py::overload_cast<>(&Transaction::getHash, py::const_), "The 256-bit hash of this transaction")
    .def_property_readonly("total_input_value", [](Transaction &tx) {
        return totalInputValue(tx);
    }, "The sum of the value of all of the inputs")
    .def_property_readonly("total_output_value", [](Transaction &tx) {
        return totalOutputValue(tx);
    }, "The sum of the value of all of the outputs")
    .def_property_readonly("fee", py::overload_cast<const Transaction &>(fee), "The fee paid by this transaction")
    .def_property_readonly("fee_per_byte", py::overload_cast<const Transaction &>(feePerByte), "The ratio of fee paid to size in bytes of this transaction")
    .def_property_readonly("op_return", py::overload_cast<const Transaction &>(getOpReturn), "If this transaction included a null data script, return its output. Otherwise return None")
    .def_property_readonly("is_coinbase", &Transaction::isCoinbase, "Return's true if this transaction is a Coinbase transaction")
    .def_property_readonly("is_coinjoin", py::overload_cast<const Transaction &>(heuristics::isCoinjoin), "Uses basic structural features to quickly decide whether this transaction might be a JoinMarket coinjoin transaction")
    .def_property_readonly("is_script_deanon", py::overload_cast<const Transaction &>(heuristics::isDeanonTx), "Returns true if this transaction's change address is deanonymized by the script types involved")
    .def_property_readonly("is_change_over", py::overload_cast<const Transaction &>(heuristics::isChangeOverTx), "Returns true if this transaction contained all inputs of one address type and all outputs of a different type")
    .def_property_readonly("is_keyset_change", py::overload_cast<const Transaction &>(heuristics::containsKeysetChange), "Returns true if this transaction contains distinct addresses which share some of the same keys, indicating that the access control structure has changed")
    .def_property_readonly("change_output", py::overload_cast<const Transaction &>(getChangeOutput), "If the change address in this transaction can be determined via the fresh address criteria, return it. Otherwise return None.")
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
