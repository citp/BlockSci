//
//  tx_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef tx_proxy_py_h
#define tx_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/chain/transaction.hpp>

void addTxProxyMethodsMain(AllProxyClasses<blocksci::Transaction> &cls);
void addTxProxyMethodsRange(AllProxyClasses<blocksci::Transaction> &cls);
void addTxProxyMethodsOptionalRange(AllProxyClasses<blocksci::Transaction> &cls);
void addTxProxyMethods(AllProxyClasses<blocksci::Transaction> &cls);

#endif /* tx_proxy_py_h */
