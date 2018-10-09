//
//  address_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef blocksci_address_py_h
#define blocksci_address_py_h

#include "method_tags.hpp"
#include "blocksci_range.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/address/equiv_address.hpp>
#include <pybind11/pybind11.h>

void init_address_type(pybind11::module &m);
void init_address(pybind11::class_<blocksci::ScriptBase> &addressCl);
void init_equiv_address(pybind11::class_<blocksci::EquivAddress> &cl);

void addAddressRangeMethods(RangeClasses<blocksci::AnyScript> &classes);


#endif /* blocksci_address_py_h */
