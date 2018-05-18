//
//  blockchain_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/30/18.
//

#ifndef blockchain_py_h
#define blockchain_py_h

#include "method_tags.hpp"

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/slice.hpp>
#include <range/v3/view/stride.hpp>

#include <pybind11/pybind11.h>

void init_data_access(pybind11::module &m);
void init_blockchain(pybind11::class_<blocksci::Blockchain> &cl);

struct AddBlockchainMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(method_tag, "address_count", &Blockchain::addressCount, "Get an upper bound of the number of address of a given type (This reflects the number of type equivlant addresses of that type).", pybind11::arg("address_type"));
        func(property_tag, "blocks", +[](Blockchain &chain) {
            return chain[{0, chain.size()}];
        }, "Returns a range of all the blocks in the chain");
        func(method_tag, "tx_with_index", +[](Blockchain &chain, uint32_t index) {
            return Transaction{index, chain.getAccess()};
        }, "This functions gets the transaction with given index.", pybind11::arg("index"));
        func(method_tag, "tx_with_hash", +[](Blockchain &chain, const std::string &hash) {
            return Transaction{hash, chain.getAccess()};
        },"This functions gets the transaction with given hash.", pybind11::arg("tx_hash"));
        func(method_tag, "address_from_index", +[](Blockchain &chain, uint32_t index, AddressType::Enum type) {
            return Address{index, type, chain.getAccess()};
        }, "Construct an address object from an address num and type", pybind11::arg("index"), pybind11::arg("type"));
        func(method_tag, "address_from_string", +[](Blockchain &chain, const std::string &addressString) -> ranges::optional<Address> {
            auto address = getAddressFromString(addressString, chain.getAccess());
            if (address) {
                return address;
            } else {
                return ranges::nullopt;
            }
        }, "Construct an address object from an address string", pybind11::arg("address_string"));
        func(method_tag, "addresses_with_prefix", +[](Blockchain &chain, const std::string &addressPrefix) {
            pybind11::list pyAddresses;
            auto addresses = getAddressesWithPrefix(addressPrefix, chain.getAccess());
            for (auto &address : addresses) {
                pyAddresses.append(address.getScript().wrapped);
            }
            return pyAddresses;
        }, "Find all addresses beginning with the given prefix", pybind11::arg("prefix"));
    }
};

#endif /* blockchain_py_h */
