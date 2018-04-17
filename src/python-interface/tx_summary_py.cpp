//
//  tx_summary_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/10/17.
//
//

#include "any_script_caster.hpp"
#include "optional_py.hpp"

#include <blocksci/chain/transaction_summary.hpp>
#include <blocksci/chain/transaction.hpp>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace blocksci;

void init_tx_summary(py::module &m) {
    py::class_<TransactionSummary>(m, "TransactionSummary", "Summary data about set of transactions")
    .def(py::self + py::self)
    .def("__add__", [](const TransactionSummary &a, const Transaction &b) {
        return a + b;
    }, py::is_operator())
    .def("average_inputs", &TransactionSummary::averageInputs)
    .def("average_outputs", &TransactionSummary::averageOutputs)
    .def("average_size", &TransactionSummary::averageSize)
    ;
}
