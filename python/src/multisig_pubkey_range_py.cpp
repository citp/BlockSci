//
//  multisig_pubkey_range_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/23/18.
//
//

#include "pubkey_range_py.hpp"

#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/multisig_pubkey_script.hpp>

using namespace blocksci;
namespace py = pybind11;

void init_multisig_pubkey_range(py::module &m) {
    setupRanges<script::MultisigPubkey>(m, "MultisigPubkey");
}