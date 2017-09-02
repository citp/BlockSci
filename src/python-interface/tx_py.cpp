//
//  tx_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/scripts/address_pointer.hpp>
#include <blocksci/uint256.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <boost/range/numeric.hpp>

namespace py = pybind11;

using namespace blocksci;

namespace pybind11 { namespace detail {
    template<typename T>
    struct type_caster<boost::optional<T>>
    {
        static handle cast(boost::optional<T> src, return_value_policy policy, handle parent)
        {
            if (src)
            {
                return type_caster<T>::cast(*src, policy, parent);
            }
            
            // if not set convert to None
            return none().inc_ref();
        }
        PYBIND11_TYPE_CASTER(boost::optional<T>, _("Optional"));
    };
}}

py::object reduceOutputsPython(const Transaction::output_range &col, py::function callable, py::object initial) {
    return boost::accumulate(col, initial, [&](py::object accumulated, const Output &output) {
        return callable(accumulated, output.getSpendingTx(), output.toAddressNum, output.getValue(), output.getType());
    });
}

py::object reduceSpentOutputsPython(const Transaction::input_range &col, py::function callable, py::object initial) {
    return boost::accumulate(col, initial, [&](py::object accumulated, const Input &input) {
        return callable(accumulated, input.linkedTxNum, input.toAddressNum, input.getValue(), input.getType());
    });
}

void init_tx(py::module &m) {
    py::class_<Transaction>(m, "Tx", "Class representing a transcation in a block")
    .def("__str__", &Transaction::getString)
    .def("__repr__", &Transaction::getString)
    .def("__eq__", &Transaction::operator==)
    .def("__hash__", [](const Transaction &tx) {
        return tx.txNum;
    })
    .def_static("tx_with_index", py::overload_cast<uint32_t>(&Transaction::txWithIndex),
                R"docstring(
                This functions gets the transaction with given index.
                
                :param int index: The index of the transation.
                :returns: Tx
                )docstring"
    )
    .def_static("tx_with_hash", py::overload_cast<uint256>(&Transaction::txWithHash))
    .def_static("tx_with_hash", py::overload_cast<std::string>(&Transaction::txWithHash))
    .def_static("txes_with_hashes", [](const std::vector<std::string> &txHashes) {
        return getTransactionsFromHashes(txHashes);
    })
    .def_property_readonly("num_txouts", &Transaction::outputCount, "The number of outputs")
    .def_property_readonly("num_txins", &Transaction::inputCount)
    .def_property_readonly("size_bytes", &Transaction::sizeBytes)
    .def_property_readonly("locktime", &Transaction::locktime)
    .def_readonly("block_height", &Transaction::blockHeight)
    .def_readonly("tx_index", &Transaction::txNum)
    .def_property_readonly("txins", &Transaction::inputs)
    .def_property_readonly("txouts", &Transaction::outputs)
    .def("hash", [](const Transaction &tx) {
        return tx.getHash().GetHex();
    })
    .def("total_in", py::overload_cast<const Transaction &>(totalIn))
    .def("total_out", totalOut)
    .def("fee", py::overload_cast<const Transaction &>(fee))
    .def("op_return", py::overload_cast<const Transaction &>(getOpReturn), py::return_value_policy::reference_internal)
    .def("op_return_data", py::overload_cast<const Transaction &>(getOpReturnData))
    .def("is_coinbase", isCoinbase)
    .def("is_coinjoin", py::overload_cast<const Transaction &>(isCoinjoin))
    .def("is_deanon_tx", py::overload_cast<const Transaction &>(isDeanonTx))
    .def("is_change_over_tx", py::overload_cast<const Transaction &>(isChangeOverTx))
    .def("is_coinjoin_extra", [](const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth) {
        py::gil_scoped_release release;
        return isCoinjoinExtra(tx, minBaseFee, percentageFee, maxDepth);
    })
    .def("is_possible_coinjoin", [](const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth) {
        py::gil_scoped_release release;
        return isPossibleCoinjoin(tx, minBaseFee, percentageFee, maxDepth);
    })
    .def("get_change_output", py::overload_cast<const Transaction &>(getChangeOutput), py::return_value_policy::reference)
    .def("get_source_txes", py::overload_cast<const Transaction &, int, const Input &>(getSourceAddresses))
    .def("get_source_txes_list", py::overload_cast<const Transaction &, int, const Input &, const std::vector<Transaction> &>( getSourceAddressesList))
    ;
    
    py::enum_<CoinJoinResult>(m, "CoinJoinResult")
    .value("True", CoinJoinResult::True)
    .value("False", CoinJoinResult::False)
    .value("Timeout", CoinJoinResult::Timeout)
    ;
    
    py::class_<Transaction::input_range>(m, "InputRange", "Class representing a range of transaction inputs")
    .def("reduce", &reduceSpentOutputsPython)
    .def("__len__", &Transaction::input_range::size)
    /// Optional sequence protocol operations
    .def("__iter__", [](const Transaction::input_range &range) { return py::make_iterator(range.begin(), range.end()); },
         py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
    .def("__getitem__", [](const Transaction::input_range &range, int64_t i) -> const Input & {
        while (i < 0) {
            i = range.size() - i;
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
    .def("reduce", &reduceOutputsPython)
    .def("__len__", &Transaction::output_range::size)
    /// Optional sequence protocol operations
    .def("__iter__", [](const Transaction::output_range &range) { return py::make_iterator(range.begin(), range.end()); },
         py::return_value_policy::reference, py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
    .def("__getitem__", [](const Transaction::output_range &range, int64_t i) -> const Output & {
        while (i < 0) {
            i = range.size() - i;
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
