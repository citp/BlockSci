//
//  block_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "block_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"
#include "ranges_py.hpp"

#include <pybind11/operators.h>

namespace py = pybind11;
using namespace blocksci;

void init_uint256(py::class_<uint256> &cl) {
    cl
    .def("__repr__", &uint256::GetHex)
    .def(py::pickle(
        [](const uint256 &hash) {
            return py::make_tuple(hash.GetHex());
        },
        [](py::tuple t) {
            if (t.size() != 1) {
                throw std::runtime_error("Invalid state!");
            }
            return uint256S(t[0].cast<std::string>());
        }
    ))
    ;
}

void init_uint160(py::class_<uint160> &cl) {
    cl
    .def("__repr__", &uint160::GetHex)
    .def(py::pickle(
        [](const uint160 &hash) {
            return py::make_tuple(hash.GetHex());
        },
        [](py::tuple t) {
            if (t.size() != 1) {
                throw std::runtime_error("Invalid state!");
            }
            return uint160S(t[0].cast<std::string>());
        }
    ))
    ;
}

void init_block(py::class_<Block> &cl) {
    addRangeMethods(cl);
    cl
    .def("__repr__", &Block::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &Block::getAccess, py::return_value_policy::reference)
    .def_property_readonly("next_block", &Block::nextBlock, "Returns the block which follows this one in the chain")
    .def_property_readonly("prev_block", &Block::prevBlock, "Returns the block which comes before this one in the chain")
    .def("net_address_type_value", py::overload_cast<const Block &>(netAddressTypeValue), "Returns a set of the net change in the utxo pool after this block split up by address type")
    .def("net_full_type_value", py::overload_cast<const Block &>(netFullTypeValue), "Returns a set of the net change in the utxo pool after this block split up by full type")
    ;

    applyMethodsToSelf(cl, AddBlockMethods{});
}
