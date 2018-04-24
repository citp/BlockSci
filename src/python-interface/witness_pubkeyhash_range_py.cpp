//
//  witness_pubkeyhash_range_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//
//

#include "pubkey_range_py.hpp"

using namespace blocksci;
namespace py = pybind11;

void init_witness_pubkeyhash_range(py::module &m) {
    setupRanges<script::WitnessPubkeyHash>(m, "WitnessPubkeyHashAddress");
}
