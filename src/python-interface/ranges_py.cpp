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
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::PUBKEY>>>(m, "AnyPubkeyAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::PUBKEYHASH>>>(m, "AnyPubkeyHashAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::MULTISIG_PUBKEY>>>(m, "AnyMultisigPubkeyAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::WITNESS_PUBKEYHASH>>>(m, "AnyWitnessPubkeyHashAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::SCRIPTHASH>>>(m, "AnyScriptHashAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::WITNESS_SCRIPTHASH>>>(m, "AnyWitnessScriptHashAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::MULTISIG>>>(m, "AnyMultisigAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::NONSTANDARD>>>(m, "AnyNonStandardAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::NULL_DATA>>>(m, "AnyOpReturnAddressRange");
    
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::PUBKEY>, ranges::category::random_access>>(m, "PubkeyAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::PUBKEYHASH>, ranges::category::random_access>>(m, "PubkeyHashAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::MULTISIG_PUBKEY>, ranges::category::random_access>>(m, "MultisigPubkeyAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::WITNESS_PUBKEYHASH>, ranges::category::random_access>>(m, "WitnessPubkeyHashAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::SCRIPTHASH>, ranges::category::random_access>>(m, "ScriptHashAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::WITNESS_SCRIPTHASH>, ranges::category::random_access>>(m, "WitnessScriptHashAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::MULTISIG>, ranges::category::random_access>>(m, "MultisigAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::NONSTANDARD>, ranges::category::random_access>>(m, "NonStandardAddressRange");
    addRangeClass<ranges::any_view<ScriptAddress<AddressType::NULL_DATA>, ranges::category::random_access>>(m, "OpReturnAddressRange");
    
    
    
}
