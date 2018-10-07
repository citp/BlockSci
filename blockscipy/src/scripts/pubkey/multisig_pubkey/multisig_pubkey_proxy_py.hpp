//
//  multisig_pubkey_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef multisig_pubkey_proxy_py_h
#define multisig_pubkey_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/chain/input.hpp>

void addMultisigPubkeyProxyMethodsMain(AllProxyClasses<blocksci::script::MultisigPubkey> &cls);
void addMultisigPubkeyProxyMethodsRange(AllProxyClasses<blocksci::script::MultisigPubkey> &cls);
void addMultisigPubkeyProxyMethodsRangeMap(AllProxyClasses<blocksci::script::MultisigPubkey> &cls);
void addMultisigPubkeyProxyMethodsRangeMapOptional(AllProxyClasses<blocksci::script::MultisigPubkey> &cls);
void addMultisigPubkeyProxyMethodsRangeMapSequence(AllProxyClasses<blocksci::script::MultisigPubkey> &cls);

void addMultisigPubkeyProxyMethods(AllProxyClasses<blocksci::script::MultisigPubkey> &cls);

#endif /* multisig_pubkey_proxy_py_h */
