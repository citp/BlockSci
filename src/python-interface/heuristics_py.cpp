//
//  heuristics_py.cpp
//  blocksci_interface
//
//  Created by Malte Möser on 10/3/17.
//

#include <blocksci/heuristics/change_address.hpp>
#include <blocksci/heuristics/tx_identification.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>

#include "optional_py.hpp"

#include <pybind11/pybind11.h>


namespace py = pybind11;
using namespace blocksci;

void init_heuristics(py::module &m) {
    auto s = m.def_submodule("heuristics");
    
    s.def("change_by_peeling_chain", heuristics::changeByPeelingChain);
    s.def("unique_change_by_peeling_chain", heuristics::uniqueChangeByPeelingChain);
    
    s.def("change_by_power_of_ten_value", heuristics::changeByPowerOfTenValue, py::arg("tx"), py::arg("digits") = 6);
    s.def("unique_change_by_power_of_ten_value", heuristics::uniqueChangeByPowerOfTenValue, py::arg("tx"), py::arg("digits") = 6);
    
    s.def("change_by_optimal_change", heuristics::changeByOptimalChange);
    s.def("unique_change_by_optimal_change", heuristics::uniqueChangeByOptimalChange);
    
    s.def("change_by_script_type", heuristics::changeByScriptType);
    s.def("unique_change_by_script_type", heuristics::uniqueChangeByScriptType);
    
    s.def("change_by_locktime", heuristics::changeByLocktime);
    s.def("unique_change_by_locktime", heuristics::uniqueChangeByLocktime);
    
    s.def("change_by_address_reuse", py::overload_cast<const Transaction &>(heuristics::changeByAddressReuse));
    s.def("unique_change_by_address_reuse", py::overload_cast<const Transaction &>(heuristics::uniqueChangeByAddressReuse));
    
    s.def("change_by_client_change_address_behavior", py::overload_cast<const Transaction &>(heuristics::changeByClientChangeAddressBehavior));
    s.def("unique_change_by_client_change_address_behavior", py::overload_cast<const Transaction &>(heuristics::uniqueChangeByClientChangeAddressBehavior));
    
    s.def_property_readonly("is_coinjoin", py::overload_cast<const Transaction &>(heuristics::isCoinjoin), "Uses basic structural features to quickly decide whether this transaction might be a JoinMarket coinjoin transaction")
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
}
