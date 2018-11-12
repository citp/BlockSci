//
//  scripthash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef scripthash_proxy_py_h
#define scripthash_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addScriptHashProxyMethodsMain(AllProxyClasses<blocksci::script::ScriptHash, ProxyAddress> &cls);
void addScriptHashProxyMethodsRange(AllProxyClasses<blocksci::script::ScriptHash, ProxyAddress> &cls);

void addScriptHashProxyMethods(AllProxyClasses<blocksci::script::ScriptHash, ProxyAddress> &cls);

#endif /* scripthash_proxy_py_h */
