//
//  pubkey_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef pubkey_proxy_py_h
#define pubkey_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addPubkeyProxyMethodsMain(AllProxyClasses<blocksci::script::Pubkey, ProxyAddress> &cls);
void addPubkeyProxyMethodsRange(AllProxyClasses<blocksci::script::Pubkey, ProxyAddress> &cls);

void addPubkeyProxyMethods(AllProxyClasses<blocksci::script::Pubkey, ProxyAddress> &cls);

#endif /* pubkey_proxy_py_h */
