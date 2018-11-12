//
//  sequence_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/11/18.
//

#ifndef generic_sequence_py_hpp
#define generic_sequence_py_hpp

#include "python_fwd.hpp"

#include <pybind11/pybind11.h>

void addCommonRangeMethods(pybind11::class_<GenericRange, GenericIterator> &cl);

#endif /* generic_sequence_py_hpp */
