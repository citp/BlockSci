//
//  pubkeyhash_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef pubkeyhash_proxy_py_h
#define pubkeyhash_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addPubkeyHashProxyMethods(AllProxyClasses<blocksci::script::PubkeyHash, ProxyAddress> &cls);

#endif /* pubkeyhash_proxy_py_h */
