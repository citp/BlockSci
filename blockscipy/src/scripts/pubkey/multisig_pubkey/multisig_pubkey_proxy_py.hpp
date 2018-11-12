//
//  multisig_pubkey_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef multisig_pubkey_proxy_py_h
#define multisig_pubkey_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addMultisigPubkeyProxyMethods(AllProxyClasses<blocksci::script::MultisigPubkey> &cls);

#endif /* multisig_pubkey_proxy_py_h */
