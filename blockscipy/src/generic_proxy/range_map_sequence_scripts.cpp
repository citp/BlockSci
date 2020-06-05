//
//  range_map_sequence_scripts.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/1/18.
//
//

#include "range_map.hpp"
#include "range_map_sequence_impl.hpp"

#include <blocksci/scripts/script_variant.hpp>

void applyProxyMapSequenceFuncsScripts(pybind11::class_<IteratorProxy, GenericProxy> &cl) {
	using namespace blocksci;
	cl
	.def("_map_sequence", mapSequence<script::Pubkey>)
	.def("_map_sequence", mapSequence<script::PubkeyHash>)
	.def("_map_sequence", mapSequence<script::WitnessPubkeyHash>)
	.def("_map_sequence", mapSequence<script::MultisigPubkey>)
	.def("_map_sequence", mapSequence<script::Multisig>)
	.def("_map_sequence", mapSequence<script::ScriptHash>)
	.def("_map_sequence", mapSequence<script::WitnessScriptHash>)
	.def("_map_sequence", mapSequence<script::OpReturn>)
	.def("_map_sequence", mapSequence<script::Nonstandard>)
	.def("_map_sequence", mapSequence<script::WitnessUnknown>)
	;
}
