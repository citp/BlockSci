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

    auto s2 = s.def_submodule("change");

    py::class_<ChangeHeuristic>(s2, "ChangeHeuristic", "Class representing a change heuristic")
    .def("__and__", &ChangeHeuristic::setIntersection, py::arg("other_heuristic"), "Return a new heuristic matching outputs that match both of the given heuristics")
    .def("__or__", &ChangeHeuristic::setUnion, py::arg("other_heuristic"), "Return a new heuristic matching outputs that match either of the given heuristics")
    .def("__sub__", &ChangeHeuristic::setDifference, py::arg("other_heuristic"), "Return a new heuristic matching outputs matched by the first heuristic unless they're matched by the second heuristic")
    .def("__call__", &ChangeHeuristic::operator(), py::arg("tx"), "Return all outputs matching the change heuristic")
    .def("change", &ChangeHeuristic::operator(), py::arg("tx"), "Return all outputs matching the change heuristic")
    .def("unique_change", &ChangeHeuristic::uniqueChange, py::arg("tx"), "If the change heuristic only matches one output return it, otherwise return none")
    ;

    s2
    .def("peeling_chain", []() { return ChangeHeuristic{PeelingChainChange{}}; }, 
        "Return a ChangeHeuristic object implementing the power of ten value heuristic: If tx is a peeling chain, returns the smaller output.")
    .def("peeling_chain", [](const Transaction &tx) { return PeelingChainChange{}(tx); }, py::arg("tx"),
        "Apply the peeling chain heuristic and return the matching output.")
    
    .def("power_of_ten_value", [](int digits) { return ChangeHeuristic{PowerOfTenChange{digits}}; }, py::arg("digits") = 6, 
        "Return a ChangeHeuristic object implementing the power of ten value heuristic: Detects possible change outputs by checking for output values that are multiples of 10^digits.")
    .def("power_of_ten_value", [](int digits, const Transaction &tx) { return PowerOfTenChange{digits}(tx); }, py::arg("digits") = 6, py::arg("tx"),
        "Apply the power of ten value heuristic to a transaction and return the set outputs that could be the change address.")
    
    .def("optimal_change", []() { return ChangeHeuristic{OptimalChangeChange{}}; },
        "Return a ChangeHeuristic object implementing the optimal change heuristic: If there exists an output that is smaller than any of the inputs it is likely the change. If a change output was larger than the smallest input, then the coin selection algorithm wouldn't need to add the input in the first place.")
    .def("optimal_change", [](const Transaction &tx) { return OptimalChangeChange{}(tx); }, py::arg("tx"),
        "Apply the optimal change heuristic to a transaction and return the set outputs that could be the change address.")
    
    .def("address_type", []() { return ChangeHeuristic{AddressTypeChange{}}; },
        "Return a ChangeHeuristic object implementing the address type heuristic: If all inputs are of one address type (e.g., P2PKH or P2SH), it is likely that the change output has the same type")
    .def("address_type", [](const Transaction &tx) { return AddressTypeChange{}(tx); }, py::arg("tx"),
        "Apply the address type heuristic to a transaction and return the set outputs that could be the change address.")

    .def("locktime", []() { return ChangeHeuristic{LocktimeChange{}}; },
        "Return a ChangeHeuristic object implementing the locktime heuristic: Bitcoin Core sets the locktime to the current block height to prevent fee sniping. If all outpus have been spent, and there is only one output that has been spent in a transaction that matches this transaction's locktime behavior, it is the change.")
    .def("locktime", [](const Transaction &tx) { return LocktimeChange{}(tx); }, py::arg("tx"),
        "Apply the locktime reuse heuristic to a transaction and return the set outputs that could be the change address.")

    .def("address_reuse", []() { return ChangeHeuristic{AddressReuseChange{}}; }, 
        "Return a ChangeHeuristic object implementing the address reuse heuristic: If input addresses appear as an output address, the client might have reused addresses for change.")
    .def("address_reuse", [](const Transaction &tx) { return AddressReuseChange{}(tx); }, py::arg("tx"),
        "Apply the address reuse heuristic to a transaction and return the set outputs that could be the change address.")

    .def("client_change_address_behavior", []() { return ChangeHeuristic{ClientChangeAddressBehaviorChange{}}; }, 
        "Return a ChangeHeuristic object implementing the client change address behavior heuristic: Most clients will generate a fresh address for the change. If an output is the first to send value to an address, it is potentially the change.")
    .def("client_change_address_behavior", [](const Transaction &tx) { return ClientChangeAddressBehaviorChange{}(tx); }, py::arg("tx"),
        "Apply the client change address behavior heuristic to a transaction and return the set outputs that could be the change address.")

    .def("legacy", []() { return ChangeHeuristic{LegacyChange{}}; },
        "Return a ChangeHeuristic object implementing the legacy heuristic: The original change address heuristic used in blocksci consisting of the intersection of the optimal change heuristic and the client address behavior heuristic")
    .def("legacy", [](const Transaction &tx) { return LegacyChange{}(tx); }, py::arg("tx"),
        "Apply the legacy heuristic to a transaction and return the set outputs that could be the change address.")

    .def("none", []() { return ChangeHeuristic{NoChange{}}; },
         "Return a ChangeHeuristic object implementing no change heuristic (effectively disabling it).")
    .def("none", [](const Transaction &tx) { return NoChange{}(tx); }, py::arg("tx"),
         "Apply the no change heuristic to a transaction and return no outputs.")
    ;
}
