//
//  heuristics_py.cpp
//  blocksci_interface
//
//  Created by Malte Möser on 10/3/17.
//

#include <blocksci/heuristics.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>

#include "optional_py.hpp"

#include <pybind11/pybind11.h>


namespace py = pybind11;
using namespace blocksci;

void init_heuristics(py::module &m) {
    auto s = m.def_submodule("heuristics");
    
    s.def("change_by_peeling_chain", heuristics::changeByPeelingChain, "If tx is a peeling chain, returns the smaller output.");
    s.def("unique_change_by_peeling_chain", heuristics::uniqueChangeByPeelingChain, "If tx is a peeling chain, returns the smaller output.");
    
    s.def("change_by_power_of_ten_value", heuristics::changeByPowerOfTenValue, py::arg("tx"), py::arg("digits") = 6,
        "Detects possible change outputs by checking for output values that are multiples of 10^digits.");
    s.def("unique_change_by_power_of_ten_value", heuristics::uniqueChangeByPowerOfTenValue, py::arg("tx"), py::arg("digits") = 6,
        "Returns a unique output from change_by_power_of_ten_value or none");
    
    s.def("change_by_optimal_change", heuristics::changeByOptimalChange,
        "If there exists an output that is smaller than any of the inputs it is likely the change. If a change output was larger than the smallest input, then the coin selection algorithm wouldn't need to add the input in the first place.");
    s.def("unique_change_by_optimal_change", heuristics::uniqueChangeByOptimalChange, "Returns a unique output from change_by_optimal_change or None");
    
    s.def("change_by_script_type", heuristics::changeByScriptType, "If all inputs are of one script type (e.g., P2PKH or P2SH), it is likely that the change output has the same type");
    s.def("unique_change_by_script_type", heuristics::uniqueChangeByScriptType, "Returns a unique output from change_by_script_type or None");
    
    s.def("change_by_locktime", heuristics::changeByLocktime, "Bitcoin Core sets the locktime to the current block height to prevent fee sniping. If all outpus have been spent, and there is only one output that has been spent in a transaction that matches this transaction's locktime behavior, it is the change.");
    s.def("unique_change_by_locktime", heuristics::uniqueChangeByLocktime, "Returns a unique output from change_by_locktime or None");
    
    s.def("change_by_address_reuse", py::overload_cast<const Transaction &>(heuristics::changeByAddressReuse), "If input addresses appear as an output address, the client might have reused addresses for change.");
    s.def("unique_change_by_address_reuse", py::overload_cast<const Transaction &>(heuristics::uniqueChangeByAddressReuse), "Returns a unique output from change_by_address_reuse or None");
    
    s.def("change_by_client_change_address_behavior", py::overload_cast<const Transaction &>(heuristics::changeByClientChangeAddressBehavior), "Most clients will generate a fresh address for the change. If an output is the first to send value to an address, it is potentially the change.");
    s.def("unique_change_by_client_change_address_behavior", py::overload_cast<const Transaction &>(heuristics::uniqueChangeByClientChangeAddressBehavior), "Returns a unique output from change_by_client_change_address_behavior or None");
    
    s.def("is_coinjoin", py::overload_cast<const Transaction &>(heuristics::isCoinjoin), "Uses basic structural features to quickly decide whether this transaction might be a JoinMarket coinjoin transaction")
    .def("is_script_deanon", py::overload_cast<const Transaction &>(heuristics::isDeanonTx), "Returns true if this transaction's change address is deanonymized by the script types involved")
    .def("is_change_over", py::overload_cast<const Transaction &>(heuristics::isChangeOverTx), "Returns true if this transaction contained all inputs of one address type and all outputs of a different type")
    .def("is_keyset_change", py::overload_cast<const Transaction &>(heuristics::containsKeysetChange), "Returns true if this transaction contains distinct addresses which share some of the same keys, indicating that the access control structure has changed")
    .def("is_definite_coinjoin", [](const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth) {
        py::gil_scoped_release release;
        return heuristics::isCoinjoinExtra(tx, minBaseFee, percentageFee, maxDepth);
    }, "This function uses subset matching in order to determine whether this transaction is a JoinMarket coinjoin. If maxDepth != 0, it limits the total number of possible subsets the algorithm will check.")
    .def("is_definite_coinjoin", [](const Transaction &tx, uint64_t minBaseFee, double percentageFee) {
        py::gil_scoped_release release;
        return heuristics::isCoinjoinExtra(tx, minBaseFee, percentageFee, 0);
    }, "This function uses subset matching in order to determine whether this transaction is a JoinMarket coinjoin.")
    ;

    s
    .def("coinjoin_txes", heuristics::getCoinjoinTransactions, "Returns a list of all transactions in the blockchain that might be JoinMarket coinjoin transactions")
    .def("possible_coinjoin_txes", heuristics::getPossibleCoinjoinTransactions, "Returns a list of all transactions in the blockchain that might be coinjoin transactions")
    .def("script_deanon_txes", heuristics::getDeanonTxes, "Return a list of all the transactions in the blockchain for which is_script_deanon returns true")
    .def("change_script_type_txes", heuristics::getChangeOverTxes, "Return a list of transactions in the blockchain for which is_change_over returns true")
    .def("keyset_change_txes", heuristics::getKeysetChangeTxes, "Return a list of transaction in the blockchain for which is_keyset_change returns true")
    ;


   py::enum_<heuristics::CoinJoinResult>(m, "CoinJoinResult")
   .value("True", heuristics::CoinJoinResult::True)
   .value("False", heuristics::CoinJoinResult::False)
   .value("Timeout", heuristics::CoinJoinResult::Timeout)
   ;
}
