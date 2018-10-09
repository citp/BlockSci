//
//  pubkeyhash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef pubkeyhash_proxy_py_h
#define pubkeyhash_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/pubkey_script.hpp>

void addPubkeyHashProxyMethods(AllProxyClasses<blocksci::script::PubkeyHash> &cls);

#endif /* pubkeyhash_proxy_py_h */
