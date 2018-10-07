//
//  scripthash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef scripthash_proxy_py_h
#define scripthash_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/chain/input.hpp>

void addScriptHashProxyMethodsMain(AllProxyClasses<blocksci::script::ScriptHash> &cls);
void addScriptHashProxyMethodsRange(AllProxyClasses<blocksci::script::ScriptHash> &cls);
void addScriptHashProxyMethodsRangeMap(AllProxyClasses<blocksci::script::ScriptHash> &cls);
void addScriptHashProxyMethodsRangeMapOptional(AllProxyClasses<blocksci::script::ScriptHash> &cls);
void addScriptHashProxyMethodsRangeMapSequence(AllProxyClasses<blocksci::script::ScriptHash> &cls);

void addScriptHashProxyMethods(AllProxyClasses<blocksci::script::ScriptHash> &cls);

#endif /* scripthash_proxy_py_h */
