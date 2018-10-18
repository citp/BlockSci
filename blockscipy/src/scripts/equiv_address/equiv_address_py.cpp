//
//  equiv_address_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "equiv_address_py.hpp"
#include "caster_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/operators.h>

#include <range/v3/view/transform.hpp>

namespace py = pybind11;
using namespace blocksci;

void init_equiv_address(py::class_<EquivAddress> &cl) {
    cl
    .def(py::self == py::self)
    .def(hash(py::self))
    .def("__repr__", &EquivAddress::toString)
    .def("__len__", [](const EquivAddress &address) { return address.size(); })
    .def("__bool__", [](const EquivAddress &address) { return address.size() == 0; })
    .def("__iter__", [](const EquivAddress &address) {
        auto transformed = address | ranges::view::transform([](const Address &address) {
            return address.getScript();
        });
        return py::make_iterator(transformed.begin(), transformed.end());
    },py::keep_alive<0, 1>())
    .def("txes", &EquivAddress::getTransactions, "Returns a list of all transactions involving these equivalent addresses")
    .def("out_txes", &EquivAddress::getOutputTransactions, "Returns a range of all transaction where these equivalent addresses were an output")
    .def("in_txes", &EquivAddress::getInputTransactions, "Returns a list of all transaction where these equivalent addresses were an input")
    .def_property_readonly("addresses", +[](EquivAddress &address) -> std::unordered_set<Address> {
        return {address.begin(), address.end()};
    }, "Returns an iterator over the addresses that make up this equivalent address");
    ;
}

void addEquivAddressRangeMethods(RangeClasses<EquivAddress> &classes) {
    addAllRangeMethods(classes);
}
