//
//  tx_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef tx_py_h
#define tx_py_h

#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/heuristics/change_address.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

#include <range/v3/view/any_view.hpp>

void init_tx(pybind11::module &m);

template <typename Class, typename FuncApplication, typename FuncDoc>
void addTransactionMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    using namespace blocksci;
    cl
    .def_property_readonly("ins", func([](const Transaction &tx) -> ranges::any_view<Input, ranges::category::random_access>  {
        return tx.inputs();
    }), func2("A list of the inputs of the transaction")) // same as below
    .def_property_readonly("inputs", func([](const Transaction &tx) -> ranges::any_view<Input, ranges::category::random_access>  {
        return tx.inputs();
    }), func2("A list of the inputs of the transaction")) // same as above
    .def_property_readonly("outs", func([](const Transaction &tx) -> ranges::any_view<Output, ranges::category::random_access>  {
        return tx.outputs();
    }), func2("A list of the outputs of the transaction")) // same as below
    .def_property_readonly("outputs", func([](const Transaction &tx) -> ranges::any_view<Output, ranges::category::random_access>  {
        return tx.outputs();
    }), func2("A list of the outputs of the transaction")) // same as above
    .def_property_readonly("output_count", func([](const Transaction &tx) -> int64_t {
        return tx.outputCount();
    }), func2("The number of outputs this transaction has"))
    .def_property_readonly("input_count", func([](const Transaction &tx) -> int64_t {
        return tx.inputCount();
    }), func2("The number of inputs this transaction has"))
    .def_property_readonly("size_bytes", func([](const Transaction &tx) -> int64_t {
        return tx.totalSize();
    }), func2("The size of this transaction in bytes"))
    .def_property_readonly("base_size", func([](const Transaction &tx) -> int64_t {
        return tx.baseSize();
    }), func2("The size of the non-segwit data in bytes"))
    .def_property_readonly("total_size", func([](const Transaction &tx) -> int64_t {
        return tx.totalSize();
    }), func2("The size all transaction data in bytes"))
    .def_property_readonly("virtual_size", func([](const Transaction &tx) -> int64_t {
        return tx.virtualSize();
    }), func2("The weight of the transaction divided by 4"))
    .def_property_readonly("weight", func([](const Transaction &tx) -> int64_t {
        return tx.weight();
    }), func2("Three times the base size plus the total size"))
    .def_property_readonly("locktime", func([](const Transaction &tx) -> int64_t {
        return tx.locktime();
    }), func2("The locktime of this transasction"))
    .def_property_readonly("block_height", func([](const Transaction &tx) -> int64_t {
        return tx.blockHeight;
    }), func2("The height of the block that this transaction was in"))
    .def_property_readonly("block_time", func([](const Transaction &tx) -> std::chrono::system_clock::time_point {
        return tx.block().getTime();
    }), func2("The time that the block containing this transaction arrived"))
    .def_property_readonly("observed_in_mempool", func([](const Transaction &tx) -> bool {
        return tx.observedInMempool();
    }), func2("Returns whether this transaction was seen in the mempool by the recorder"))
    .def_property_readonly("time_seen", func([](const Transaction &tx) -> ranges::optional<std::chrono::system_clock::time_point> {
        return tx.getTimeSeen();
    }), func2("If recorded by the mempool recorder, the time that this transaction was first seen by your node"))
    .def_property_readonly("block", func([](const Transaction &tx) -> Block {
        return tx.block();
    }), func2("The block that this transaction was in"))
    .def_property_readonly("index", func([](const Transaction &tx) -> int64_t {
        return tx.txNum;
    }), func2("The internal index of this transaction"))
    .def_property_readonly("hash", func([](const Transaction &tx) -> uint256 {
        return tx.getHash();
    }), func2("The 256-bit hash of this transaction"))
    .def_property_readonly("input_value", func([](const Transaction &tx) -> int64_t {
        return totalInputValue(tx);
    }), func2("The sum of the value of all of the inputs"))
    .def_property_readonly("output_value", func([](const Transaction &tx) -> int64_t {
        return totalOutputValue(tx);
    }), func2("The sum of the value of all of the outputs"))
    .def_property_readonly("fee", func([](const Transaction &tx) -> int64_t {
        return fee(tx);
    }), func2("The fee paid by this transaction"))
    .def("fee_per_byte", func([](const Transaction &tx, const std::string &sizeMeasure) -> int64_t {
        auto txFee = fee(tx);
        if (sizeMeasure == "total") {
            return txFee / tx.totalSize();
        } else if (sizeMeasure == "base") {
            return txFee / tx.baseSize();
        } else if(sizeMeasure == "weight") {
            return txFee / tx.weight();
        } else if(sizeMeasure == "virtual") {
            return txFee / tx.virtualSize();
        } else {
            throw std::invalid_argument{"Size measure must be one of total, base, weight, or virtual"};
        }
    }), pybind11::arg("size_measure") = "virtual", func2("The ratio of fee paid to size of this transaction. By default this uses veritual size, but passing total, base, weight, or virtual let's you choose what measure of size you want"))
    .def_property_readonly("op_return", func([](const Transaction &tx) -> ranges::optional<Output> {
        return getOpReturn(tx);
    }), func2("If this transaction included a null data address, return its output. Otherwise return None"))
    .def_property_readonly("is_coinbase", func([](const Transaction &tx) -> bool {
        return tx.isCoinbase();
    }), func2("Return's true if this transaction is a Coinbase transaction"))
    .def_property_readonly("change_output", func([](const Transaction &tx) -> ranges::optional<Output> {
        return heuristics::uniqueChangeByLegacyHeuristic(tx);
    }), func2("If the change address in this transaction can be determined via the fresh address criteria, return it. Otherwise return None."))
    ;
}

#endif /* tx_py_h */
