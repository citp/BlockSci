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

template <typename T>
auto addTxRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addTransactionMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each transaction: " << docstring;
        return strdup(ss.str().c_str());
    });
    addTransactionRangeMethods(cl, [](auto &range, auto func) {
        return func(range);
    });
    return cl;
}

template <typename T>
auto addOptionalTxRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addTransactionMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each transaction: " << docstring;
        return strdup(ss.str().c_str());
    });
    addTransactionRangeMethods(cl, [](auto &range, auto func) {
        return func(range | flatMapOptionals);
    });
    return cl;
}

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
    
    addTxRange<ranges::any_view<Transaction>>(m, "AnyTxRange");
    addTxRange<ranges::any_view<Transaction, ranges::category::random_access>>(m, "TxRange");
    addOptionalTxRange<ranges::any_view<ranges::optional<Transaction>>>(m, "AnyOptionaTxRange");
    addOptionalTxRange<ranges::any_view<ranges::optional<Transaction>, ranges::category::random_access>>(m, "OptionaTxRange");
}
