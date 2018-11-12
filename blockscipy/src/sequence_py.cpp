//
//  sequence_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/11/18.
//

#include "sequence_py.hpp"
#include "generic_sequence.hpp"

void addCommonRangeMethods(pybind11::class_<GenericRange, GenericIterator> &cl) {
    cl
    .def("__bool__", [](GenericRange &range) {
        return !range.empty();
        
    })
    .def("__len__", [](GenericRange &range) {
        return range.size();
    })
    ;
}