//
//  output_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef output_py_h
#define output_py_h

#include <pybind11/pybind11.h>

#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/any_view.hpp>

void init_output(pybind11::module &m);

struct AddOutputMethods {
    using FuncDoc = std::function<const char *(std::string)>;

    FuncDoc func2;

    AddOutputMethods(FuncDoc funcDoc_) : func2(std::move(funcDoc_)) {}

    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        namespace py = pybind11;
        using namespace blocksci;
        cl
        .def_property_readonly("address", func(&blocksci::Output::getAddress), func2("This address linked to this output"))
        .def_property_readonly("value", func(&blocksci::Output::getValue), func2("The value in base currency attached to this output"))
        .def_property_readonly("address_type", func(&blocksci::Output::getType), func2("The address type of the output"))
        .def_property_readonly("is_spent", func(&blocksci::Output::isSpent), func2("Returns whether this output has been spent"))
        .def_property_readonly("spending_tx_index", func(&blocksci::Output::getSpendingTxIndex), func2("Returns the index of the tranasction which spent this output or 0 if it is unspent"))
        .def_property_readonly("spending_tx", func(&blocksci::Output::getSpendingTx), func2("Returns the transaction that spent this output or None if it is unspent"))
        .def_property_readonly("tx", func(&blocksci::Output::transaction), func2("The transaction that contains this input"))
        .def_property_readonly("block", func(&blocksci::Output::block), func2("The block that contains this input"))
        .def_property_readonly("index", func(&blocksci::Output::outputIndex), func2("The output index inside this output's transaction"))
        .def_property_readonly("tx_index", func(&blocksci::Output::txIndex), func2("The tx index of this output's transaction"))
        ;
    }
};

#endif /* output_py_h */
