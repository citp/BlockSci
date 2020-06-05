//
//  multisig_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef multisig_proxy_py_h
#define multisig_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addMultisigProxyMethods(AllProxyClasses<blocksci::script::Multisig, ProxyAddress> &cls);

#endif /* multisig_proxy_py_h */
