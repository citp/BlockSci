//
//  tx_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "tx_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"

#include <blocksci/chain/access.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

void init_tx(py::class_<Transaction> &cl) {
    applyMethodsToSelf(cl, AddTransactionMethods{});
    
    cl
    .def("__str__", &Transaction::toString)
    .def("__repr__", &Transaction::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", [](const Transaction &tx) {
        return Access{&tx.getAccess()};
    })
    .def(py::init([](uint32_t index, blocksci::Blockchain &chain) {
        return Transaction{index, chain.getAccess()};
    }), "This functions gets the transaction with given index.")
    .def(py::init([](const std::string hash, blocksci::Blockchain &chain) {
        return Transaction{hash, chain.getAccess()};
    }), "This functions gets the transaction with given hash.")
    ;
}
