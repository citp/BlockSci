//
//  address_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef blocksci_equiv_address_py_h
#define blocksci_equiv_address_py_h

#include "blocksci_range.hpp"

#include <blocksci/address/equiv_address.hpp>

#include <pybind11/pybind11.h>

void init_equiv_address(pybind11::class_<blocksci::EquivAddress> &cl);
void addEquivAddressRangeMethods(RangeClasses<blocksci::EquivAddress> &classes);

#endif /* blocksci_equiv_address_py_h */
