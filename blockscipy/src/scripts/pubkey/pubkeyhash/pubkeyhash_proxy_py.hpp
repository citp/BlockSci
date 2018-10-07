//
//  pubkeyhash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef pubkeyhash_proxy_py_h
#define pubkeyhash_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/chain/input.hpp>

void addPubkeyHashProxyMethodsMain(AllProxyClasses<blocksci::script::PubkeyHash> &cls);
void addPubkeyHashProxyMethodsRange(AllProxyClasses<blocksci::script::PubkeyHash> &cls);
void addPubkeyHashProxyMethodsRangeMap(AllProxyClasses<blocksci::script::PubkeyHash> &cls);
void addPubkeyHashProxyMethodsRangeMapOptional(AllProxyClasses<blocksci::script::PubkeyHash> &cls);
void addPubkeyHashProxyMethodsRangeMapSequence(AllProxyClasses<blocksci::script::PubkeyHash> &cls);

void addPubkeyHashProxyMethods(AllProxyClasses<blocksci::script::PubkeyHash> &cls);

#endif /* pubkeyhash_proxy_py_h */
