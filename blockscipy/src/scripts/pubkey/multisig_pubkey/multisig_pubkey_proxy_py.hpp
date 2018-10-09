//
//  multisig_pubkey_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef multisig_pubkey_proxy_py_h
#define multisig_pubkey_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/multisig_pubkey_script.hpp>

void addMultisigPubkeyProxyMethods(AllProxyClasses<blocksci::script::MultisigPubkey> &cls);

#endif /* multisig_pubkey_proxy_py_h */
