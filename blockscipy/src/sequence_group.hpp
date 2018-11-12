//
//  sequence_group.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/11/18.
//

#ifndef sequence_group_hpp
#define sequence_group_hpp

#include "python_fwd.hpp"

#include <pybind11/pybind11.h>

void addAllGroupMethods(pybind11::class_<GenericSequence> &cl);

#endif /* sequence_group_hpp */
