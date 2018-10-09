//
//  pubkey_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef pubkey_proxy_py_h
#define pubkey_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/pubkey_script.hpp>

void addPubkeyProxyMethodsMain(AllProxyClasses<blocksci::script::Pubkey> &cls);
void addPubkeyProxyMethodsRange(AllProxyClasses<blocksci::script::Pubkey> &cls);

void addPubkeyProxyMethods(AllProxyClasses<blocksci::script::Pubkey> &cls);

#endif /* pubkey_proxy_py_h */
