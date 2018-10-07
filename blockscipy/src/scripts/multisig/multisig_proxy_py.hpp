//
//  multisig_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef multisig_proxy_py_h
#define multisig_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/chain/input.hpp>

void addMultisigProxyMethodsMain(AllProxyClasses<blocksci::script::Multisig> &cls);
void addMultisigProxyMethodsRange(AllProxyClasses<blocksci::script::Multisig> &cls);
void addMultisigProxyMethodsRangeMap(AllProxyClasses<blocksci::script::Multisig> &cls);
void addMultisigProxyMethodsRangeMapOptional(AllProxyClasses<blocksci::script::Multisig> &cls);
void addMultisigProxyMethodsRangeMapSequence(AllProxyClasses<blocksci::script::Multisig> &cls);

void addMultisigProxyMethods(AllProxyClasses<blocksci::script::Multisig> &cls);

#endif /* multisig_proxy_py_h */
