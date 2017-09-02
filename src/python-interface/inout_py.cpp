//
//  input_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include <blocksci/scripts/address_pointer.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/chain/transaction.hpp>


#include <pybind11/pybind11.h>

namespace py = pybind11;

using namespace blocksci;

void init_inout(py::module &m) {
    py::class_<OutputPointer>(m, "OutputPointer", "Class representing a pointer to an output")
    .def("__repr__", &OutputPointer::toString)
    .def("__eq__", &OutputPointer::operator==)
    .def("__hash__", [] (const OutputPointer &pointer) {
        uint64_t hash = pointer.txNum;
        hash <<= 32;
        hash += static_cast<uint64_t>(pointer.outputNum);
        return hash;
    })
    .def_readonly("tx_index_from", &OutputPointer::txNum)
    .def_readonly("output_index_from", &OutputPointer::outputNum)
    .def("output", static_cast<const Output &(OutputPointer::*)() const>(&OutputPointer::getOutput), py::return_value_policy::reference)
    .def("transaction", static_cast<Transaction(OutputPointer::*)() const>(&OutputPointer::getTransaction))
    ;
    
    py::class_<Inout> inout(m, "Inout", "Class representing an input or output");
    
    inout
    .def("__eq__", &Inout::operator==)
    .def("__hash__", [] (const Inout &output) {
        return reinterpret_cast<size_t>(&output);
    })
    .def_readonly("linkedTxNum", &Inout::linkedTxNum)
    .def_property_readonly("address_pointer", &Inout::getAddressPointer)
    .def_property_readonly("satoshis", &Inout::getValue)
    .def_property_readonly("script_type", &Inout::getType)
    ;
    
    py::class_<Input>(m, "Input", inout, "Class representing a transaction input")
    .def("__repr__", &Input::toString)
    .def_property_readonly("spent_tx", static_cast<Transaction(Input::*)() const>(&Input::getSpentTx))
    ;
    
    py::class_<Output>(m, "Output", inout, "Class representing a transaction output")
    .def("__repr__", &Output::toString)
    .def_property_readonly("tx_index_to", static_cast<uint32_t(Output::*)() const>(&Output::getSpendingTxIndex))
    .def("is_spent", static_cast<bool(Output::*)() const>(&Output::isSpent))
    .def("spendingTx", static_cast<boost::optional<Transaction>(Output::*)() const>(&Output::getSpendingTx))
    ;

}
