//
//  heuristics_py.cpp
//  blocksci_interface
//
//  Created by Malte Möser on 10/3/17.
//

#include "caster_py.hpp"
#include "proxy.hpp"
#include "proxy_create.hpp"
#include "proxy_utils.hpp"

#include <blocksci/heuristics.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/transaction.hpp>

namespace py = pybind11;
using namespace blocksci;
using namespace blocksci::heuristics;

struct Heuristics {};
struct Change {};

void init_heuristics(py::module &m) {

    py::enum_<heuristics::CoinJoinResult>(m, "CoinJoinResult")
    .value("True", heuristics::CoinJoinResult::True)
    .value("False", heuristics::CoinJoinResult::False)
    .value("Timeout", heuristics::CoinJoinResult::Timeout)
    ;

    py::class_<Heuristics> cl(m, "heuristics");

    cl
    .def_property_readonly_static("is_coinjoin", [](pybind11::object &) -> Proxy<bool> {
        return lift(makeSimpleProxy<Transaction>(), heuristics::isCoinjoin);
    }, "Uses basic structural features to quickly decide whether this transaction might be a JoinMarket coinjoin transaction")
    .def_property_readonly_static("is_address_deanon", [](pybind11::object &) -> Proxy<bool> {
        return lift(makeSimpleProxy<Transaction>(), heuristics::isDeanonTx);
    }, "Returns true if this transaction's change address is deanonymized by the address types involved")
    .def_property_readonly_static("is_change_over", [](pybind11::object &) -> Proxy<bool> {
        return lift(makeSimpleProxy<Transaction>(), heuristics::isChangeOverTx);
    }, "Returns true if this transaction contained all inputs of one address type and all outputs of a different type")
    .def_property_readonly_static("is_keyset_change", [](pybind11::object &) -> Proxy<bool> {
        return lift(makeSimpleProxy<Transaction>(), heuristics::containsKeysetChange);
    }, "Returns true if this transaction contains distinct addresses which share some of the same keys, indicating that the access control structure has changed")
    .def_static("is_possible_coinjoin", [](int64_t minBaseFee, double percentageFee, size_t maxDepth) -> Proxy<int64_t> {
        return lift(makeSimpleProxy<Transaction>(), [=](const Transaction &tx) -> int64_t {
            py::gil_scoped_release release;
            return static_cast<int64_t>(heuristics::isPossibleCoinjoin(tx, minBaseFee, percentageFee, maxDepth));
        });
    }, py::arg("min_base_fee"), py::arg("percentage_fee"), py::arg("max_depth") = 0, "This function uses subset matching in order to determine whether this transaction is a JoinMarket coinjoin. If maxDepth != 0, it limits the total number of possible subsets the algorithm will check.")
    .def_static("is_definite_coinjoin", [](int64_t minBaseFee, double percentageFee, size_t maxDepth) -> Proxy<int64_t> {
        return lift(makeSimpleProxy<Transaction>(), [=](const Transaction &tx) -> int64_t {
            py::gil_scoped_release release;
            return static_cast<int64_t>(heuristics::isCoinjoinExtra(tx, minBaseFee, percentageFee, maxDepth));
        });
    }, py::arg("min_base_fee"), py::arg("percentage_fee"), py::arg("max_depth") = 0, "This function uses subset matching in order to determine whether this transaction is a JoinMarket coinjoin. If maxDepth != 0, it limits the total number of possible subsets the algorithm will check.")
    ;

    cl
    .def_static("poison_tainted_outputs", heuristics::getPoisonTainted, py::arg("outputs"), py::arg("max_block_height") = -1, py::arg("taint_fee") = true, "Returns the list of current UTXOs poison tainted by this output")
    .def_static("haircut_tainted_outputs", heuristics::getHaircutTainted, py::arg("outputs"), py::arg("max_block_height") = -1, py::arg("taint_fee") = true, "Returns the list of current UTXOs haircut tainted by this output")
    ;

    py::class_<Change> s2(cl, "change");

    py::class_<ChangeHeuristic>(s2, "ChangeHeuristic", "Class representing a change heuristic")
    .def("__and__", &ChangeHeuristic::setIntersection, py::arg("other_heuristic"), "Return a new heuristic matching outputs that match both of the given heuristics")
    .def("__or__", &ChangeHeuristic::setUnion, py::arg("other_heuristic"), "Return a new heuristic matching outputs that match either of the given heuristics")
    .def("__sub__", &ChangeHeuristic::setDifference, py::arg("other_heuristic"), "Return a new heuristic matching outputs matched by the first heuristic unless they're matched by the second heuristic")
    .def("__call__", &ChangeHeuristic::operator(), py::arg("tx"), "Return all outputs matching the change heuristic")
    .def("change", &ChangeHeuristic::operator(), py::arg("tx"), "Return all outputs matching the change heuristic")
    .def_property_readonly("unique_change", [](ChangeHeuristic &ch) -> Proxy<ranges::optional<Output>> {
        return lift(makeSimpleProxy<Transaction>(), [ch](const Transaction &tx) {
            return ch.uniqueChange(tx);
        });
    }, "Returns a proxy object which takes a tx as input. If the change heuristic only matches one output return it, otherwise return none")
    ;

    s2
    .def_property_readonly_static("peeling_chain", [](pybind11::object &) { return ChangeHeuristic{PeelingChainChange{}}; }, 
        "Return a ChangeHeuristic object implementing the power of ten value heuristic: If tx is a peeling chain, returns the smaller output.")
    
    .def_static("power_of_ten_value", [](int digits) { return ChangeHeuristic{PowerOfTenChange{digits}}; }, py::arg("digits") = 6, 
        "Return a ChangeHeuristic object implementing the power of ten value heuristic: Detects possible change outputs by checking for output values that are multiples of 10^digits.")
    
    .def_property_readonly_static("optimal_change", [](pybind11::object &) { return ChangeHeuristic{OptimalChangeChange{}}; },
        "Return a ChangeHeuristic object implementing the optimal change heuristic: If there exists an output that is smaller than any of the inputs it is likely the change. If a change output was larger than the smallest input, then the coin selection algorithm wouldn't need to add the input in the first place.")
    
    .def_property_readonly_static("address_type", [](pybind11::object &) { return ChangeHeuristic{AddressTypeChange{}}; },
        "Return a ChangeHeuristic object implementing the address type heuristic: If all inputs are of one address type (e.g., P2PKH or P2SH), it is likely that the change output has the same type")

    .def_property_readonly_static("locktime", [](pybind11::object &) { return ChangeHeuristic{LocktimeChange{}}; },
        "Return a ChangeHeuristic object implementing the locktime heuristic: Bitcoin Core sets the locktime to the current block height to prevent fee sniping. If all outpus have been spent, and there is only one output that has been spent in a transaction that matches this transaction's locktime behavior, it is the change.")

    .def_property_readonly_static("address_reuse", [](pybind11::object &) { return ChangeHeuristic{AddressReuseChange{}}; }, 
        "Return a ChangeHeuristic object implementing the address reuse heuristic: If input addresses appear as an output address, the client might have reused addresses for change.")

    .def_property_readonly_static("client_change_address_behavior", [](pybind11::object &) { return ChangeHeuristic{ClientChangeAddressBehaviorChange{}}; }, 
        "Return a ChangeHeuristic object implementing the client change address behavior heuristic: Most clients will generate a fresh address for the change. If an output is the first to send value to an address, it is potentially the change.")

    .def_property_readonly_static("legacy", [](pybind11::object &) { return ChangeHeuristic{LegacyChange{}}; },
        "Return a ChangeHeuristic object implementing the legacy heuristic: The original change address heuristic used in blocksci consisting of the intersection of the optimal change heuristic and the client address behavior heuristic")
    ;
}
