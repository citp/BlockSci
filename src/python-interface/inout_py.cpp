//
//  input_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "optional_py.hpp"

#include <blocksci/address/address.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/inout_pointer.hpp>
#include <blocksci/chain/transaction.hpp>


#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

using namespace blocksci;

void init_inout(py::module &m) {
    py::class_<OutputPointer>(m, "OutputPointer", "Class representing a pointer to an output")
    .def("__repr__", &OutputPointer::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("tx_index", &OutputPointer::txNum)
    .def_readonly("out_index", &OutputPointer::inoutNum)
    ;

    py::class_<InputPointer>(m, "InputPointer", "Class representing a pointer to an input")
    .def("__repr__", &InputPointer::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_readonly("tx_index", &InputPointer::txNum)
    .def_readonly("int_index", &InputPointer::inoutNum)
    ;
        
    py::class_<Input>(m, "Input", "Class representing a transaction input")
    .def("__repr__", &Input::toString)
    .def(py::init<InputPointer>())
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("address", &Input::getAddress, "This address linked to this inout")
    .def_property_readonly("value", &Input::getValue, "The value in base currency attached to this inout")
    .def_property_readonly("script_type", &Input::getType, "The type of the address")
    .def_property_readonly("spent_tx_index", &Input::spentTxIndex, "The index of the transaction that this input spent")
    .def_property_readonly("spent_tx", &Input::getSpentTx, "The transaction that this input spent")
    ;
    
    py::class_<Output>(m, "Output", "Class representing a transaction output")
    .def("__repr__", &Output::toString)
    .def(py::init<OutputPointer>())
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("address", &Output::getAddress, "This address linked to this inout")
    .def_property_readonly("value", &Output::getValue, "The value in base currency attached to this inout")
    .def_property_readonly("script_type", &Output::getType, "The type of the address")
    .def_property_readonly("is_spent", &Output::isSpent, "Returns whether this output has been spent")
    .def_property_readonly("spending_tx_index", &Output::getSpendingTxIndex, "Returns the index of the tranasction which spent this output or 0 if it is unspent")
    .def_property_readonly("spending_tx", &Output::getSpendingTx, "Returns the transaction that spent this output or None if it is unspent")
    ;

}
