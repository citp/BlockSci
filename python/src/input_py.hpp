//
//  input_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef input_py_hpp
#define input_py_hpp

#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>

#include <pybind11/pybind11.h>

void init_input(pybind11::module &m);

struct AddInputMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddInputMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        namespace py = pybind11;
        using namespace blocksci;
        cl
        .def_property_readonly("address", func(&blocksci::Input::getAddress), func2("The address linked to this input"))
        .def_property_readonly("value", func(&blocksci::Input::getValue), func2("The value in base currency attached to this input"))
        .def_property_readonly("address_type", func(&blocksci::Input::getType), func2("The address type of the input"))
        .def_property_readonly("sequence_num", func(&blocksci::Input::sequenceNumber), func2("The sequence number of the input"))
        .def_property_readonly("spent_tx_index", func(&blocksci::Input::spentTxIndex), func2("The index of the transaction that this input spent"))
        .def_property_readonly("spent_tx", func(&blocksci::Input::getSpentTx), func2("The transaction that this input spent"))
        .def_property_readonly("age", func(&blocksci::Input::age), func2("The number of blocks between the spent output and this input"))
        .def_property_readonly("tx", func(&blocksci::Input::transaction), func2("The transaction that contains this input"))
        .def_property_readonly("block", func(&blocksci::Input::block), func2("The block that contains this input"))
        .def_property_readonly("index", func(&blocksci::Input::inputIndex), func2("The index index inside this input's transaction"))
        .def_property_readonly("tx_index", func(&blocksci::Input::txIndex), func2("The tx index of this input's transaction"))
        ;
    }
};

#endif /* input_py_hpp */
