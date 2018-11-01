//
//  range_map_optional_range.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "range_map.hpp"
#include "range_map_optional_impl.hpp"

#include <blocksci/address/equiv_address.hpp>
#include <blocksci/cluster/cluster.hpp>


void applyProxyMapOptionalFuncsScripts(pybind11::class_<IteratorProxy> &cl) {
	using namespace blocksci;
	cl
	.def("_map_optional", mapOptional<script::Pubkey>)
	.def("_map_optional", mapOptional<script::PubkeyHash>)
	.def("_map_optional", mapOptional<script::WitnessPubkeyHash>)
	.def("_map_optional", mapOptional<script::MultisigPubkey>)
	.def("_map_optional", mapOptional<script::Multisig>)
	.def("_map_optional", mapOptional<script::ScriptHash>)
	.def("_map_optional", mapOptional<script::WitnessScriptHash>)
	.def("_map_optional", mapOptional<script::OpReturn>)
	.def("_map_optional", mapOptional<script::Nonstandard>)
	.def("_map_optional", mapOptional<script::WitnessUnknown>)
	;
}
