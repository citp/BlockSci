//
//  blocksci.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "optional_py.hpp"
#include "variant_py.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>

namespace py = pybind11;

void init_address(py::module &m);
void init_address_index(py::module &m);
void init_tx(py::module &m);
void init_tx_summary(py::module &m);
void init_inout(py::module &m);
void init_block(py::module &m);
void init_blockchain(py::module &m);
void init_heuristics(py::module &m);

PYBIND11_MODULE(blocksci_interface, m) {
    init_address(m);
    init_blockchain(m);
    init_block(m);
    init_tx(m);
    init_tx_summary(m);
    init_inout(m);
    init_heuristics(m);
}
