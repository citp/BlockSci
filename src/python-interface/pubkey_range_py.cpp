//
//  pubkey_range_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//
//

#include "pubkey_range_py.hpp"

using namespace blocksci;
namespace py = pybind11;

void init_pubkey_range(py::module &m) {
    setupRanges<script::Pubkey>(m, "PubkeyAddress");
}