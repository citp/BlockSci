//
//  ranges_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#include "ranges_py.hpp"

#include <blocksci/chain/output.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/blockchain.hpp>

#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace blocksci;

void init_ranges(py::module &m) {
	addRangeClass<ranges::any_view<Block>>(m, "AnyBlockRange");
 
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::PUBKEY>>>(m, "AnyPubkeyScriptRange");
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::SCRIPTHASH>>>(m, "AnyScripthashScriptRange");
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::MULTISIG>>>(m, "AnyMultisigScriptRange");
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::NONSTANDARD>>>(m, "AnyNonStandardScriptRange");
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::NULL_DATA>>>(m, "AnyNullDataScriptRange");
    
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::PUBKEY>, ranges::category::random_access | ranges::category::sized>>(m, "PubkeyScriptRange");
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::SCRIPTHASH>, ranges::category::random_access | ranges::category::sized>>(m, "ScripthashScriptRange");
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::MULTISIG>, ranges::category::random_access | ranges::category::sized>>(m, "MultisigScriptRange");
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::NONSTANDARD>, ranges::category::random_access | ranges::category::sized>>(m, "NonStandardScriptRange");
    addRangeClass<ranges::any_view<ScriptAddress<ScriptType::Enum::NULL_DATA>, ranges::category::random_access | ranges::category::sized>>(m, "NullDataScriptRange");
    
    
    
}
