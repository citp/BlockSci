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
}
