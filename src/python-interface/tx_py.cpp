//
//  tx_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "optional_py.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/bitcoin_uint256.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <boost/range/numeric.hpp>

namespace py = pybind11;

using namespace blocksci;

void init_tx(py::module &m) {
    py::class_<Transaction>(m, "Tx", "Class representing a transaction in a block")
    .def("__str__", &Transaction::getString)
    .def("__repr__", &Transaction::getString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_static("tx_with_index", py::overload_cast<uint32_t>(&Transaction::txWithIndex),
                R"docstring(
                This functions gets the transaction with given index.
                
                :param int index: The index of the transation.
                :returns: Tx
                )docstring"
    )
    .def_static("tx_with_hash", py::overload_cast<std::string>(&Transaction::txWithHash))
    .def_property_readonly("num_outs", &Transaction::outputCount, "The number of outputs this transaction has")
    .def_property_readonly("num_ins", &Transaction::inputCount, "The number of inputs this transaction has")
    .def_property_readonly("size", &Transaction::sizeBytes, "The size of this transaction in bytes")
    .def_property_readonly("locktime", &Transaction::locktime, "The locktime of this transasction")
    .def_readonly("block_height", &Transaction::blockHeight, "The height of the block that this transaction was in")
    .def_property_readonly("block", py::overload_cast<>(&Transaction::block, py::const_), "The block that this transaction was in")
    .def_readonly("index", &Transaction::txNum, "The internal index of this transaction")
    .def_property_readonly("ins", &Transaction::inputs, "A list of the inputs of the transaction")
    .def_property_readonly("outs", &Transaction::outputs, "A list of the outputs of the transaction")
    .def_property_readonly("hash", py::overload_cast<>(&Transaction::getHash, py::const_), "The 256-bit hash of this transaction")
    .def_property_readonly("value_in", py::overload_cast<const Transaction &>(totalIn), "The sum of the value of all of the inputs")
    .def_property_readonly("value_out", py::overload_cast<const Transaction &>(totalOut), "The sum of the value of all of the outputs")
    .def_property_readonly("fee", py::overload_cast<const Transaction &>(fee), "The fee paid by this transaction")
    .def_property_readonly("fee_per_byte", py::overload_cast<const Transaction &>(feePerByte), "The ratio of fee paid to size in bytes of this transaction")
    .def_property_readonly("op_return", py::overload_cast<const Transaction &>(getOpReturn), py::return_value_policy::reference_internal, "If this transaction included a null data script, return its output. Otherwise return None")
    .def_property_readonly("is_coinbase", isCoinbase, "Return's true if this transaction is a Coinbase transaction")
    .def_property_readonly("is_coinjoin", py::overload_cast<const Transaction &>(isCoinjoin), "Uses basic structural features to quickly decide whether this transaction might be a JoinMarket coinjoin transaction")
    .def_property_readonly("is_script_deanon", py::overload_cast<const Transaction &>(isDeanonTx), "Returns true if this transaction's change address is deanonymized by the script types involved")
    .def_property_readonly("is_change_over", py::overload_cast<const Transaction &>(isChangeOverTx), "Returns true if this transaction contained all inputs of one address type and all outputs of a different type")
    .def_property_readonly("is_keyset_change", py::overload_cast<const Transaction &>(containsKeysetChange), "Returns true if this transaction contains distinct addresses which share some of the same keys, indicating that the access control structure has changed")
    .def_property_readonly("is_definite_coinjoin", [](const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth) {
        py::gil_scoped_release release;
        return isCoinjoinExtra(tx, minBaseFee, percentageFee, maxDepth);
    }, py::arg("maxDepth") = 0, "This function uses subset matching in order to determine whether this transaction is a JoinMarket coinjoin. If maxDepth != 0, it limits the total number of possible subsets the algorithm will check.")
    .def_property_readonly("change_output", py::overload_cast<const Transaction &>(getChangeOutput), py::return_value_policy::reference, "If the change address in this transaction can be determined via the fresh address criteria, return it. Otherwise return None.")
    ;
    
    py::enum_<CoinJoinResult>(m, "CoinJoinResult")
    .value("True", CoinJoinResult::True)
    .value("False", CoinJoinResult::False)
    .value("Timeout", CoinJoinResult::Timeout)
    ;
    
    py::class_<Transaction::input_range>(m, "InputRange", "Class representing a range of transaction inputs")
    .def("__len__", &Transaction::input_range::size)
    .def("__iter__", [](const Transaction::input_range &range) { return py::make_iterator(range.begin(), range.end()); },
         py::keep_alive<0, 1>())
    .def("__getitem__", [](const Transaction::input_range &range, int64_t i) -> const Input & {
        while (i < 0) {
            i += range.size();
        }
        uint64_t posIndex = static_cast<uint64_t>(i);
        if (posIndex >= range.size())
            throw py::index_error();
        return range[i];
    }, py::return_value_policy::reference)
    .def("__getitem__", [](const Transaction::input_range &range, py::slice slice) -> py::list {
        size_t start, stop, step, slicelength;
        if (!slice.compute(range.size(), &start, &stop, &step, &slicelength))
            throw py::error_already_set();
        py::list txList;
        for (size_t i=0; i<slicelength; ++i) {
            txList.append(range[start]);
            start += step;
        }
        return txList;
    });
    
    py::class_<Transaction::output_range>(m, "OutputRange", "Class representing a range of transaction outputs")
    .def("__len__", &Transaction::output_range::size)
    .def("__iter__", [](const Transaction::output_range &range) { return py::make_iterator(range.begin(), range.end()); },
         py::return_value_policy::reference, py::keep_alive<0, 1>())
    .def("__getitem__", [](const Transaction::output_range &range, int64_t i) -> const Output & {
        while (i < 0) {
            i += range.size();
        }
        uint64_t posIndex = static_cast<uint64_t>(i);
        if (posIndex >= range.size())
            throw py::index_error();
        return range[i];
    }, py::return_value_policy::reference)
    .def("__getitem__", [](const Transaction::output_range &range, py::slice slice) -> py::list {
        size_t start, stop, step, slicelength;
        if (!slice.compute(range.size(), &start, &stop, &step, &slicelength))
            throw py::error_already_set();
        py::list txList;
        for (size_t i=0; i<slicelength; ++i) {
            txList.append(range[start]);
            start += step;
        }
        return txList;
    });
}
