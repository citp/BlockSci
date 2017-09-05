//
//  input_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include <blocksci/address/address.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/output_pointer.hpp>
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
    .def("out", static_cast<const Output &(OutputPointer::*)() const>(&OutputPointer::getOutput), py::return_value_policy::reference)
    .def("tx", static_cast<Transaction(OutputPointer::*)() const>(&OutputPointer::getTransaction))
    ;
    
    py::class_<Inout> inout(m, "Inout", "Class representing an input or output");
    
    inout
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("address", &Inout::getAddress, "This address linked to this inout")
    .def_property_readonly("value", &Inout::getValue, "The value in base currency attached to this inout")
    .def_property_readonly("script_type", &Inout::getType, "The type of the address")
    ;
    
    py::class_<Input>(m, "Input", inout, "Class representing a transaction input")
    .def("__repr__", &Input::toString)
    .def_property_readonly("spent_tx_index", &Input::spentTxIndex, "The index of the transaction that this input spent")
    .def_property_readonly("spent_tx", static_cast<Transaction(Input::*)() const>(&Input::getSpentTx), "The transaction that this input spent")
    ;
    
    py::class_<Output>(m, "Output", inout, "Class representing a transaction output")
    .def("__repr__", &Output::toString)
    .def_property_readonly("is_spent", static_cast<bool(Output::*)() const>(&Output::isSpent), "Returns whether this output has been spent")
    .def_property_readonly("spending_tx_index", static_cast<uint32_t(Output::*)() const>(&Output::getSpendingTxIndex), "Returns the index of the tranasction which spent this output or 0 if it is unspent")
    .def_property_readonly("spending_tx", static_cast<boost::optional<Transaction>(Output::*)() const>(&Output::getSpendingTx), "Returns the transaction that spent this output or None if it is unspent")
    ;

}
