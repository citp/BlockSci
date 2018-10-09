//
//  blockchain_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/30/18.
//

#ifndef blockchain_py_h
#define blockchain_py_h

#include "method_tags.hpp"

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/slice.hpp>
#include <range/v3/view/stride.hpp>

#include <pybind11/pybind11.h>

void init_data_access(pybind11::module &m);
void init_blockchain(pybind11::class_<blocksci::Blockchain> &cl);

#endif /* blockchain_py_h */
