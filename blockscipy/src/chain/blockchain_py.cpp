//
//  blockchain_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "blockchain_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/access.hpp>
#include <blocksci/scripts/script_range.hpp>
#include <blocksci/cluster/cluster.hpp>

namespace py = pybind11;

using namespace blocksci;

void init_blockchain(py::class_<Blockchain> &cl) {
    cl
    .def("__len__", [](Blockchain &chain) { return chain.size(); })
    .def("__bool__", [](Blockchain &range) { return !ranges::empty(range); })
    .def("__iter__", [](Blockchain &chain) { return pybind11::make_iterator(chain.begin(), chain.end()); },
         pybind11::keep_alive<0, 1>())
    .def("__getitem__", [](Blockchain &chain, int64_t posIndex) {
        auto chainSize = static_cast<int64_t>(chain.size());
        if (posIndex < 0) {
            posIndex += chainSize;
        }
        if (posIndex < 0 || posIndex >= chainSize) {
            throw pybind11::index_error();
        }
        return chain[posIndex];
    }, py::arg("index"))
    .def("__getitem__", [](Blockchain &chain, pybind11::slice slice) -> ranges::any_view<Block, ranges::category::random_access | ranges::category::sized> {
        size_t start, stop, step, slicelength;
        if (!slice.compute(chain.size(), &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();

        if (step != 1) {
            throw std::runtime_error{"Cannot slice blockchain with step size not equal to one"};
        }

        return chain[{static_cast<BlockHeight>(start), static_cast<BlockHeight>(stop)}];
    }, py::arg("slice"))
    ;

    cl
    .def(py::init<std::string>())
    .def(py::init<std::string, BlockHeight>())
    .def_property_readonly("data_location", &Blockchain::dataLocation, "Returns the location of the data directory that this Blockchain object represents.")
    .def("reload", &Blockchain::reload, "Reload the blockchain to make new blocks visible (Invalidates current BlockSci objects)")
    .def("addresses", [](Blockchain &chain, AddressType::Enum type) {
        return scriptsRange(type, chain.getAccess());
    }, py::arg("address_type"), "Return a range of all addresses of the given type")
    .def("most_valuable_addresses", mostValuableAddresses, "Get a list of the top 100 most valuable addresses")
    .def("_segment_indexes", [](Blockchain &chain, BlockHeight start, BlockHeight stop, unsigned int cpuCount) {
        auto segments = chain[{start, stop}].segment(cpuCount);
        std::vector<std::pair<BlockHeight, BlockHeight>> ret;
        ret.reserve(segments.size());
        for (auto segment : segments) {
            ret.emplace_back(segment.sl.start, segment.sl.stop);
        }
        return ret;
    })
    ;

    applyMethodsToSelf(cl, AddBlockchainMethods{});
}

void init_data_access(py::module &m) {
    py::class_<Access> (m, "_DataAccess", "Private class for accessing blockchain data")
    .def("tx_with_index", &Access::txWithIndex, "This functions gets the transaction with given index.")
    .def("tx_with_hash", &Access::txWithHash, "This functions gets the transaction with given hash.")
    .def("address_from_index", &Access::addressFromIndex, "Construct an address object from an address num and type")
    .def("address_from_string", [](Access &access, const std::string &addressString) -> ranges::optional<AnyScript> {
        auto address = access.addressFromString(addressString);
        if (address) {
            return address->getScript().wrapped;
        } else {
            return ranges::nullopt;
        }
    }, "Construct an address object from an address string")
    .def("addresses_with_prefix", [](Access &access, const std::string &addressPrefix) {
        py::list pyAddresses;
        auto addresses = access.addressesWithPrefix(addressPrefix);
        for (auto &address : addresses) {
            pyAddresses.append(address.getScript().wrapped);
        }
        return pyAddresses;
    }, "Find all addresses beginning with the given prefix")
    ;
}
