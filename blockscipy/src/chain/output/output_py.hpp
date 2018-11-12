//
//  output_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef output_py_h
#define output_py_h

#include "python_fwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>

#include <pybind11/pybind11.h>

void init_output(pybind11::class_<blocksci::Output> &cl);
void addOutputRangeMethods(RangeClasses<blocksci::Output> &classes);

#endif /* output_py_h */
