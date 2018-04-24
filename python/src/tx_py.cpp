//
//  tx_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "tx_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"
#include "range_apply_py.hpp"
#include "self_apply_py.hpp"

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

#include <pybind11/operators.h>

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
    return cl;
}

template <typename T>
auto addOptionalTxRange(py::module &m, const std::string &name) {
    auto cl = addOptionalRangeClass<T>(m, name);
    addTransactionMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each transaction: " << docstring;
        return strdup(ss.str().c_str());
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
    .def(py::init([](uint32_t index, blocksci::Blockchain &chain) {
        return Transaction{index, chain.getAccess()};
    }), R"docstring(
         This functions gets the transaction with given index.
         
         :param int index: The index of the transation.
         :returns: Tx
         )docstring")
    .def(py::init([](const std::string hash, blocksci::Blockchain &chain) {
        return Transaction{hash, chain.getAccess()};
    }), R"docstring(
         This functions gets the transaction with given hash.
         
         :param string index: The hash of the transation.
         :returns: Tx
         )docstring")
    ;
    
    addTxRange<ranges::any_view<Transaction>>(m, "AnyTxRange");
    addTxRange<ranges::any_view<Transaction, ranges::category::random_access>>(m, "TxRange");
    addOptionalTxRange<ranges::any_view<ranges::optional<Transaction>>>(m, "AnyOptionalTxRange");
    addOptionalTxRange<ranges::any_view<ranges::optional<Transaction>, ranges::category::random_access>>(m, "OptionalTxRange");
}