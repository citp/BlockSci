//
//  heuristics_py.cpp
//  blocksci_interface
//
//  Created by Malte Möser on 10/3/17.
//

#include "caster_py.hpp"

#include <blocksci/heuristics.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/transaction.hpp>

namespace py = pybind11;
using namespace blocksci;
using namespace blocksci::heuristics;

void init_heuristics(py::module &m) {
    auto s = m.def_submodule("heuristics");

    py::enum_<heuristics::CoinJoinResult>(s, "CoinJoinResult")
    .value("True", heuristics::CoinJoinResult::True)
    .value("False", heuristics::CoinJoinResult::False)
    .value("Timeout", heuristics::CoinJoinResult::Timeout)
    ;

    py::class_<ChangeHeuristic>(m, "ChangeHeuristic", "Class representing a change heuristic")
    .def("__and__", &ChangeHeuristic::setIntersection)
    .def("__or__", &ChangeHeuristic::setUnion)
    .def("__sub__", &ChangeHeuristic::setDifference)
    .def("__call__", &ChangeHeuristic::operator())
    .def("change", &ChangeHeuristic::operator())
    .def("unique_change", &ChangeHeuristic::uniqueChange)
    ;

    s.def("change_by_peeling_chain", heuristics::changeByPeelingChain, py::arg("tx"), "If tx is a peeling chain, returns the smaller output.");
    s.def("unique_change_by_peeling_chain", heuristics::uniqueChangeByPeelingChain, py::arg("tx"), "If tx is a peeling chain, returns the smaller output.");
    
    s.def("change_by_power_of_ten_value", heuristics::changeByPowerOfTenValue, py::arg("tx"), py::arg("digits") = 6,
        "Detects possible change outputs by checking for output values that are multiples of 10^digits.");
    s.def("unique_change_by_power_of_ten_value", heuristics::uniqueChangeByPowerOfTenValue, py::arg("tx"), py::arg("digits") = 6,
        "Returns a unique output from change_by_power_of_ten_value or none");
    
    s.def("change_by_optimal_change", heuristics::changeByOptimalChange, py::arg("tx"),
        "If there exists an output that is smaller than any of the inputs it is likely the change. If a change output was larger than the smallest input, then the coin selection algorithm wouldn't need to add the input in the first place.");
    s.def("unique_change_by_optimal_change", heuristics::uniqueChangeByOptimalChange, py::arg("tx"), "Returns a unique output from change_by_optimal_change or None");
    
    s.def("change_by_address_type", heuristics::changeByAddressType, py::arg("tx"), "If all inputs are of one address type (e.g., P2PKH or P2SH), it is likely that the change output has the same type");
    s.def("unique_change_by_address_type", heuristics::uniqueChangeByAddressType, py::arg("tx"), "Returns a unique output from change_by_address_type or None");
    
    s.def("change_by_locktime", heuristics::changeByLocktime, py::arg("tx"), "Bitcoin Core sets the locktime to the current block height to prevent fee sniping. If all outpus have been spent, and there is only one output that has been spent in a transaction that matches this transaction's locktime behavior, it is the change.");
    s.def("unique_change_by_locktime", heuristics::uniqueChangeByLocktime, py::arg("tx"), "Returns a unique output from change_by_locktime or None");
    
    s.def("change_by_address_reuse", heuristics::changeByAddressReuse, py::arg("tx"), "If input addresses appear as an output address, the client might have reused addresses for change.");
    s.def("unique_change_by_address_reuse", heuristics::uniqueChangeByAddressReuse, py::arg("tx"), "Returns a unique output from change_by_address_reuse or None");
    
    s.def("change_by_client_change_address_behavior", heuristics::changeByClientChangeAddressBehavior, py::arg("tx"), "Most clients will generate a fresh address for the change. If an output is the first to send value to an address, it is potentially the change.");
    s.def("unique_change_by_client_change_address_behavior", heuristics::uniqueChangeByClientChangeAddressBehavior, py::arg("tx"), "Returns a unique output from change_by_client_change_address_behavior or None");
    
    auto s2 = s.def_submodule("change");
    s2
    .def("peeling_chain", []() { return ChangeHeuristic{PeelingChainChange{}}; })
    .def("peeling_chain", [](const Transaction &tx) { return PeelingChainChange{}(tx); }, py::arg("tx"))
    .def("power_of_ten_value", [](int digits) { return ChangeHeuristic{PowerOfTenChange{digits}}; }, py::arg("digits") = 6)
    .def("power_of_ten_value", [](int digits, const Transaction &tx) { return PowerOfTenChange{digits}(tx); }, py::arg("digits") = 6, py::arg("tx"))
    .def("optimal_change", []() { return ChangeHeuristic{OptimalChangeChange{}}; })
    .def("optimal_change", [](const Transaction &tx) { return OptimalChangeChange{}(tx); }, py::arg("tx"))
    .def("address_type", []() { return ChangeHeuristic{AddressTypeChange{}}; })
    .def("address_type", [](const Transaction &tx) { return AddressTypeChange{}(tx); }, py::arg("tx"))
    .def("locktime", []() { return ChangeHeuristic{LocktimeChange{}}; })
    .def("locktime", [](const Transaction &tx) { return LocktimeChange{}(tx); }, py::arg("tx"))
    .def("address_reuse", []() { return ChangeHeuristic{AddressReuseChange{}}; })
    .def("address_reuse", [](const Transaction &tx) { return AddressReuseChange{}(tx); }, py::arg("tx"))
    .def("client_change_address_behavior", []() { return ChangeHeuristic{ClientChangeAddressBehaviorChange{}}; })
    .def("client_change_address_behavior", [](const Transaction &tx) { return ClientChangeAddressBehaviorChange{}(tx); }, py::arg("tx"))
    .def("legacy", []() { return ChangeHeuristic{LegacyChange{}}; })
    .def("legacy", [](const Transaction &tx) { return LegacyChange{}(tx); }, py::arg("tx"))
    ;

    s.def("is_coinjoin", heuristics::isCoinjoin, py::arg("tx"), "Uses basic structural features to quickly decide whether this transaction might be a JoinMarket coinjoin transaction")
    .def("is_address_deanon", heuristics::isDeanonTx, py::arg("tx"), "Returns true if this transaction's change address is deanonymized by the address types involved")
    .def("is_change_over", heuristics::isChangeOverTx, py::arg("tx"), "Returns true if this transaction contained all inputs of one address type and all outputs of a different type")
    .def("is_keyset_change", heuristics::containsKeysetChange, py::arg("tx"), "Returns true if this transaction contains distinct addresses which share some of the same keys, indicating that the access control structure has changed")
    .def("is_definite_coinjoin", [](const Transaction &tx, int64_t minBaseFee, double percentageFee, size_t maxDepth) {
        py::gil_scoped_release release;
        return heuristics::isCoinjoinExtra(tx, minBaseFee, percentageFee, maxDepth);
    }, py::arg("tx"), py::arg("min_base_fee"), py::arg("percentage_fee"), py::arg("max_depth") = 0, "This function uses subset matching in order to determine whether this transaction is a JoinMarket coinjoin. If maxDepth != 0, it limits the total number of possible subsets the algorithm will check.")
    ;

    s
    .def("coinjoin_txes", heuristics::getCoinjoinTransactions, py::arg("chain"), py::arg("start"), py::arg("end"), "Returns a list of all transactions in the blockchain that might be JoinMarket coinjoin transactions")
    .def("possible_coinjoin_txes", heuristics::getPossibleCoinjoinTransactions, py::arg("chain"), py::arg("min_base_fee"), py::arg("percentage_fee"), py::arg("max_depth") = 0, "Returns a list of all transactions in the blockchain that might be coinjoin transactions")
    .def("address_deanon_txes", heuristics::getDeanonTxes, py::arg("chain"), py::arg("start"), py::arg("end"), "Return a list of all the transactions in the blockchain for which is_address_deanon returns true")
    .def("change_over_txes", heuristics::getChangeOverTxes, py::arg("chain"), py::arg("start"), py::arg("end"), "Return a list of transactions in the blockchain for which is_change_over returns true")
    .def("keyset_change_txes", heuristics::getKeysetChangeTxes, py::arg("chain"), py::arg("start"), py::arg("end"), "Return a list of transaction in the blockchain for which is_keyset_change returns true")
    ;
    
    s
    .def("poison_tainted_outputs", heuristics::getPoisonTainted, py::arg("output"), py::arg("tainted_value"), "Returns the list of current UTXOs poison tainted by this output")
    .def("haircut_tainted_outputs", heuristics::getHaircutTainted, py::arg("output"), py::arg("tainted_value"), "Returns the list of current UTXOs haircut tainted by this output")
    .def("fifo_tainted_outputs", heuristics::getFifoTainted, py::arg("output"), py::arg("tainted_value"), "Returns the list of current UTXOs FIFO tainted by this output")
    ;
}
