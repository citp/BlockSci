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
    .def("__getitem__", [](Blockchain &chain, pybind11::slice slice) -> ranges::any_view<decltype(chain[0]), ranges::category::random_access | ranges::category::sized> {
        size_t start, stop, step, slicelength;
        if (!slice.compute(chain.size(), &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();
        
        auto blockRange = chain.blocks();
        auto subset =  ranges::view::slice(blockRange,
                                           static_cast<ranges::range_difference_type_t<decltype(blockRange)>>(start),
                                           static_cast<ranges::range_difference_type_t<decltype(blockRange)>>(stop));
        auto strided = subset | ranges::view::stride(step);
        return strided;
    }, py::arg("slice"))
    ;

    cl
    .def(py::init<std::string>())
    .def(py::init<DataConfiguration>())
    .def_property_readonly("_config", [](Blockchain &chain) -> DataConfiguration { return chain.getAccess().config; }, "Returns the configuration settings for this blockchain")
    .def("_segment", segmentChain, "Divide the blockchain into the given number of chunks with roughly the same number of transactions in each")
    .def("_segment_indexes", segmentChainIndexes, "Return a list of [start, end] block height pairs representing chunks with roughly the same number of transactions in each")
    .def("reload", &Blockchain::reload, "Reload the blockchain to make new blocks visible (Invalidates current BlockSci objects)")
    .def("addresses", [](Blockchain &chain, AddressType::Enum type) {
        return chain.scripts(type);
    }, py::arg("address_type"), "Return a range of all addresses of the given type")
    .def("most_valuable_addresses", mostValuableAddresses, "Get a list of the top 100 most valuable addresses")
    ;

    applyMethodsToSelf(cl, AddBlockchainMethods{});
}

void init_data_access(py::module &m) {
    
    py::class_<DataConfiguration> (m, "DataConfiguration", "This class holds the configuration data about a blockchain instance")
    .def(py::pickle(
        [](const DataConfiguration &config) {
            return py::make_tuple(config.dataDirectory, config.errorOnReorg, config.blocksIgnored);
        },
        [](py::tuple t) {
            if (t.size() != 3) {
                throw std::runtime_error("Invalid state!");
            }
            return DataConfiguration(t[0].cast<std::string>(), t[1].cast<bool>(), t[2].cast<BlockHeight>());
        }
    ))
    ;

    py::class_<DataAccess> (m, "_DataAccess", "Private class for accessing blockchain data")
    .def("tx_with_index", [](DataAccess &access, uint32_t index) {
        return Transaction{index, access};
    }, "This functions gets the transaction with given index.")
    .def("tx_with_hash", [](DataAccess &access, const std::string &hash) {
        return Transaction{hash, access};
    }, "This functions gets the transaction with given hash.")
    .def("address_from_index", [](DataAccess &access, uint32_t index, AddressType::Enum type) -> AnyScript {
        return Address{index, type, access}.getScript().wrapped;
    }, "Construct an address object from an address num and type")
    .def("address_from_string", [](DataAccess &access, const std::string &addressString) -> ranges::optional<AnyScript> {
        auto address = getAddressFromString(addressString, access);
        if (address) {
            return address->getScript().wrapped;
        } else {
            return ranges::nullopt;
        }
    }, "Construct an address object from an address string")
    .def("addresses_with_prefix", [](DataAccess &access, const std::string &addressPrefix) {
        py::list pyAddresses;
        auto addresses = getAddressesWithPrefix(addressPrefix, access);
        for (auto &address : addresses) {
            pyAddresses.append(address.getScript().wrapped);
        }
        return pyAddresses;
    }, "Find all addresses beginning with the given prefix")
    ;
}
