//
//  multisig_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef multisig_proxy_py_h
#define multisig_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/multisig_script.hpp>

void addMultisigProxyMethods(AllProxyClasses<blocksci::script::Multisig> &cls);

#endif /* multisig_proxy_py_h */
