//
//  heuristics_py.cpp
//  blocksci_interface
//
//  Created by Malte Möser on 10/3/17.
//

#include <blocksci/heuristics/change_address.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>

#include "optional_py.hpp"

#include <pybind11/pybind11.h>


namespace py = pybind11;
using namespace blocksci;

void init_heuristics(py::module &m) {
    auto s = m.def_submodule("heuristics");
    
    s.def("change_by_peeling_chain", heuristics::changeByPeelingChain, py::return_value_policy::reference);
    s.def("unique_change_by_peeling_chain", heuristics::uniqueChangeByPeelingChain, py::return_value_policy::reference);
    
    s.def("change_by_power_of_ten_value", heuristics::changeByPowerOfTenValue, py::arg("tx"), py::arg("digits") = 6);
    s.def("unique_change_by_power_of_ten_value", heuristics::uniqueChangeByPowerOfTenValue, py::arg("tx"), py::arg("digits") = 6, py::return_value_policy::reference);
    
    s.def("change_by_optimal_change", heuristics::changeByOptimalChange, py::return_value_policy::reference);
    s.def("unique_change_by_optimal_change", heuristics::uniqueChangeByOptimalChange, py::return_value_policy::reference);
    
    s.def("change_by_script_type", heuristics::changeByScriptType, py::return_value_policy::reference);
    s.def("unique_change_by_script_type", heuristics::uniqueChangeByScriptType, py::return_value_policy::reference);
    
    s.def("change_by_locktime", heuristics::changeByLocktime, py::return_value_policy::reference);
    s.def("unique_change_by_locktime", heuristics::uniqueChangeByLocktime, py::return_value_policy::reference);
    
    s.def("change_by_address_reuse", heuristics::changeByAddressReuse, py::return_value_policy::reference);
    s.def("unique_change_by_address_reuse", heuristics::uniqueChangeByAddressReuse, py::return_value_policy::reference);
    
    s.def("change_by_client_change_address_behavior", heuristics::changeByClientChangeAddressBehavior, py::return_value_policy::reference);
    s.def("unique_change_by_client_change_address_behavior", heuristics::uniqueChangeByClientChangeAddressBehavior, py::return_value_policy::reference);
}
