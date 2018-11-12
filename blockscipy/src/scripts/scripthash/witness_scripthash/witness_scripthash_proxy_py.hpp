//
//  witness_scripthash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef witness_scripthash_proxy_py_h
#define witness_scripthash_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addWitnessScriptHashProxyMethodsMain(AllProxyClasses<blocksci::script::WitnessScriptHash> &cls);
void addWitnessScriptHashProxyMethodsRange(AllProxyClasses<blocksci::script::WitnessScriptHash> &cls);

void addWitnessScriptHashProxyMethods(AllProxyClasses<blocksci::script::WitnessScriptHash> &cls);

#endif /* witness_scripthash_proxy_py_h */
