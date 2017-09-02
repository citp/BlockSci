//
//  blocksci.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <blocksci/scripts/address_index.hpp>
#include <blocksci/scripts/address_pointer.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/chain/transaction.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace blocksci;

using ListCasterBase = pybind11::detail::list_caster<std::vector<const Output *>, Output *>;
namespace pybind11 { namespace detail {
    template<> struct type_caster<std::vector<const Output *>> : ListCasterBase {
        static handle cast(const std::vector<const Output *> &src, return_value_policy, handle parent) {
            return ListCasterBase::cast(src, return_value_policy::reference, parent);
        }
        static handle cast(const std::vector<const Output *> *src, return_value_policy pol, handle parent) {
            return cast(*src, pol, parent);
        }
    };
}}

void init_address_index(py::module &m) {
     py::class_<AddressIndex>(m, "AddressIndex", "Class managing the address index")
    .def("output_pointers", &AddressIndex::getOutputPointers)
     .def("outputs", static_cast<std::vector<const Output *>(AddressIndex::*)(const AddressPointer &) const>(&AddressIndex::getOutputs))
     .def("output_transactions", static_cast<std::vector<Transaction>(AddressIndex::*)(const AddressPointer &) const>(&AddressIndex::getOutputTransactions))
     ;
}
