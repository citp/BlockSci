//
//  proxy_flow_functions_scripts.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/20/18.
//
//

#include "proxy_flow_functions_impl.hpp"

#include <blocksci/scripts/script_variant.hpp>

void addProxyFlowFunctionsScripts(pybind11::module &m, pybind11::module &pm) {
	using namespace blocksci;

	addProxyFlowFunctions<script::Pubkey>(m, pm);
	addProxyFlowFunctions<script::PubkeyHash>(m, pm);
	addProxyFlowFunctions<script::WitnessPubkeyHash>(m, pm);
	addProxyFlowFunctions<script::MultisigPubkey>(m, pm);
	addProxyFlowFunctions<script::Multisig>(m, pm);
	addProxyFlowFunctions<script::ScriptHash>(m, pm);
	addProxyFlowFunctions<script::WitnessScriptHash>(m, pm);
	addProxyFlowFunctions<script::OpReturn>(m, pm);
	addProxyFlowFunctions<script::Nonstandard>(m, pm);
	addProxyFlowFunctions<script::WitnessUnknown>(m, pm);

}
