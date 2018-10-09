//
//  scripthash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef scripthash_proxy_py_h
#define scripthash_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/scripthash_script.hpp>

void addScriptHashProxyMethodsMain(AllProxyClasses<blocksci::script::ScriptHash> &cls);
void addScriptHashProxyMethodsRange(AllProxyClasses<blocksci::script::ScriptHash> &cls);

void addScriptHashProxyMethods(AllProxyClasses<blocksci::script::ScriptHash> &cls);

#endif /* scripthash_proxy_py_h */
