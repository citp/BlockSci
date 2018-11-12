//
//  witness_pubkeyhash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef witness_pubkeyhash_proxy_py_h
#define witness_pubkeyhash_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addWitnessPubkeyHashProxyMethodsMain(AllProxyClasses<blocksci::script::WitnessPubkeyHash, ProxyAddress> &cls);
void addWitnessPubkeyHashProxyMethodsRange(AllProxyClasses<blocksci::script::WitnessPubkeyHash, ProxyAddress> &cls);

void addWitnessPubkeyHashProxyMethods(AllProxyClasses<blocksci::script::WitnessPubkeyHash, ProxyAddress> &cls);

#endif /* witness_pubkeyhash_proxy_py_h */
