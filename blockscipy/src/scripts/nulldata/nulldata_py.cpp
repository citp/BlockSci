//
//  nulldata_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#include "nulldata_py.hpp"
#include "caster_py.hpp"
#include "self_apply_py.hpp"

using namespace blocksci;
namespace py = pybind11;

void init_nulldata(py::class_<script::OpReturn> &cl) {
    cl
    .def("__repr__", &script::OpReturn::toString)
    .def("__str__", &script::OpReturn::toPrettyString)
    ;

    applyMethodsToSelf(cl, AddOpReturnMethods{});
}

