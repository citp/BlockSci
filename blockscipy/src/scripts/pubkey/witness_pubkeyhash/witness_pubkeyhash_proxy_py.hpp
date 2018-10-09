//
//  witness_pubkeyhash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef witness_pubkeyhash_proxy_py_h
#define witness_pubkeyhash_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/pubkey_script.hpp>

void addWitnessPubkeyHashProxyMethodsMain(AllProxyClasses<blocksci::script::WitnessPubkeyHash> &cls);
void addWitnessPubkeyHashProxyMethodsRange(AllProxyClasses<blocksci::script::WitnessPubkeyHash> &cls);

void addWitnessPubkeyHashProxyMethods(AllProxyClasses<blocksci::script::WitnessPubkeyHash> &cls);

#endif /* witness_pubkeyhash_proxy_py_h */
