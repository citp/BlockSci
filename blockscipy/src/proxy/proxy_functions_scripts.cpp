//
//  proxy_functions_scripts.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/17/18.
//
//

#include "proxy_functions_impl.hpp"

#include <blocksci/scripts/script_variant.hpp>

void addProxyFunctionsScripts(pybind11::module &m, pybind11::module &pm) {
	using namespace blocksci;

	addProxyFunctions<script::Pubkey>(m, pm);
	addProxyFunctions<script::PubkeyHash>(m, pm);
	addProxyFunctions<script::WitnessPubkeyHash>(m, pm);
	addProxyFunctions<script::MultisigPubkey>(m, pm);
	addProxyFunctions<script::Multisig>(m, pm);
	addProxyFunctions<script::ScriptHash>(m, pm);
	addProxyFunctions<script::WitnessScriptHash>(m, pm);
	addProxyFunctions<script::OpReturn>(m, pm);
	addProxyFunctions<script::Nonstandard>(m, pm);
	addProxyFunctions<script::WitnessUnknown>(m, pm);

}
