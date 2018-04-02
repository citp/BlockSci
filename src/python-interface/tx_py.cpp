//
//  tx_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "tx_py.hpp"
#include "optional_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/chrono.h>

namespace py = pybind11;

using namespace blocksci;

void init_tx(py::module &m) {
    
    py::class_<Transaction> txClass(m, "Tx", "Class representing a transaction in a block");
    addTransactionMethods(txClass, [](auto func) {
        return applyMethodsToSelf<Transaction>(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    txClass
    .def("__str__", &Transaction::toString)
    .def("__repr__", &Transaction::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &Transaction::getAccess, py::return_value_policy::reference)
    .def(py::init([](uint32_t index, const blocksci::Blockchain &chain) {
        return Transaction{index, chain.getAccess()};
    }), R"docstring(
         This functions gets the transaction with given index.
         
         :param int index: The index of the transation.
         :returns: Tx
         )docstring")
    .def(py::init([](const std::string hash, const blocksci::Blockchain &chain) {
        return Transaction{hash, chain.getAccess()};
    }), R"docstring(
         This functions gets the transaction with given hash.
         
         :param string index: The hash of the transation.
         :returns: Tx
         )docstring")
    .def_property_readonly("ins", [](const Transaction &tx) -> ranges::any_view<Input, ranges::category::random_access>  {
        return tx.inputs();
    }, "A list of the inputs of the transaction") // same as below
    .def_property_readonly("inputs", [](const Transaction &tx) -> ranges::any_view<Input, ranges::category::random_access>  {
        return tx.inputs();
    }, "A list of the inputs of the transaction") // same as above
    .def_property_readonly("outs", [](const Transaction &tx) -> ranges::any_view<Output, ranges::category::random_access>  {
        return tx.outputs();
    }, "A list of the outputs of the transaction") // same as below
    .def_property_readonly("outputs", [](const Transaction &tx) -> ranges::any_view<Output, ranges::category::random_access>  {
        return tx.outputs();
    }, "A list of the outputs of the transaction") // same as above
    ;
    
    auto txRangeClass = addRangeClass<ranges::any_view<Transaction>>(m, "AnyTxRange");
    addTransactionMethods(txRangeClass, [](auto func) {
        return applyMethodsToRange<ranges::any_view<Transaction>>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each transaction: " << docstring;
        return strdup(ss.str().c_str());
    });
    addTransactionRangeMethods(txRangeClass, [](ranges::any_view<Transaction> &view, auto func) {
        return func(view);
    });
    
    auto txRangeClass2 = addRangeClass<ranges::any_view<Transaction, ranges::category::random_access>>(m, "TxRange");
    addTransactionMethods(txRangeClass2, [](auto func) {
        return applyMethodsToRange<ranges::any_view<Transaction, ranges::category::random_access>>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each transaction: " << docstring;
        return strdup(ss.str().c_str());
    });
    addTransactionRangeMethods(txRangeClass2, [](ranges::any_view<Transaction, ranges::category::random_access> &view, auto func) {
        return func(view);
    });
}
