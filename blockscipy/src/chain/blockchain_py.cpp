//
//  blockchain_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "blockchain_py.hpp"
#include "caster_py.hpp"
#include "sequence.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/access.hpp>
#include <blocksci/scripts/script_range.hpp>
#include <blocksci/cluster/cluster.hpp>

namespace py = pybind11;

using namespace blocksci;

template <AddressType::Enum type>
using PythonScriptRange = Range<ScriptAddress<type>>;
using PythonScriptRangeVariant = to_variadic_t<to_address_tuple_t<PythonScriptRange>, mpark::variant>;

namespace {
    template<blocksci::AddressType::Enum type>
    struct PythonScriptRangeFunctor {
        static PythonScriptRangeVariant f(blocksci::DataAccess &access) {
            auto scriptCount = getScriptCount(type, access);
            return PythonScriptRange<type>{ranges::view::ints(uint32_t{1}, scriptCount + 1) | ranges::view::transform([&](uint32_t scriptNum) {
                return ScriptAddress<type>(scriptNum, access);
            })};
        }
    };
}

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
    .def("__getitem__", [](Blockchain &chain, pybind11::slice slice) -> Range<Block> {
        size_t start, stop, step, slicelength;
        if (!slice.compute(chain.size(), &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();

        if (step != 1) {
            throw std::runtime_error{"Cannot slice blockchain with step size not equal to one"};
        }

        return ranges::any_view<Block, random_access_sized>{chain[{static_cast<BlockHeight>(start), static_cast<BlockHeight>(stop)}]};
    }, py::arg("slice"))
    ;

    cl
    .def(py::init<std::string>())
    .def(py::init<std::string, BlockHeight>())
    .def_property_readonly("data_location", &Blockchain::dataLocation, "Returns the location of the data directory that this Blockchain object represents.")
    .def_property_readonly("config_location", &Blockchain::configLocation, "Returns the location of the configuration file that this Blockchain object represents.")
    .def("reload", &Blockchain::reload, "Reload the blockchain to make new blocks visible (Invalidates current BlockSci objects)")
    .def("addresses", [](Blockchain &chain, AddressType::Enum type) {
        static constexpr auto table = make_dynamic_table<AddressType, PythonScriptRangeFunctor>();
        auto index = static_cast<size_t>(type);
        return table.at(index)(chain.getAccess());
    }, py::arg("address_type"), "Return a range of all addresses of the given type")
    
    .def("address_count",
        &Blockchain::addressCount,
        "Get an upper bound of the number of address of a given type (This reflects the number of type equivlant addresses of that type).",
        pybind11::arg("address_type")
    )
    .def_property_readonly("blocks",
        +[](Blockchain &chain) -> Range<Block> {
        return ranges::any_view<Block, random_access_sized>{chain};
    }, "Returns a range of all the blocks in the chain")
    .def("tx_with_index", [](Blockchain &chain, uint32_t index) {
        return Transaction{index, chain.getAccess()};
    }, "This functions gets the transaction with given index.", pybind11::arg("index"))
    .def("tx_with_hash", [](Blockchain &chain, const std::string &hash) {
        return Transaction{hash, chain.getAccess()};
    },"This functions gets the transaction with given hash.", pybind11::arg("tx_hash"))
    .def("address_from_index", [](Blockchain &chain, uint32_t index, AddressType::Enum type) {
        return Address{index, type, chain.getAccess()};
    }, "Construct an address object from an address num and type", pybind11::arg("index"), pybind11::arg("type"))
    .def("address_from_string", [](Blockchain &chain, const std::string &addressString) -> ranges::optional<Address> {
        auto address = getAddressFromString(addressString, chain.getAccess());
        if (address) {
            return address;
        } else {
            return ranges::nullopt;
        }
    }, "Construct an address object from an address string", pybind11::arg("address_string"))
    .def("addresses_with_prefix", [](Blockchain &chain, const std::string &addressPrefix) {
        pybind11::list pyAddresses;
        auto addresses = getAddressesWithPrefix(addressPrefix, chain.getAccess());
        for (auto &address : addresses) {
            pyAddresses.append(address.getScript().wrapped);
        }
        return pyAddresses;
    }, "Find all addresses beginning with the given prefix", pybind11::arg("prefix"))
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
