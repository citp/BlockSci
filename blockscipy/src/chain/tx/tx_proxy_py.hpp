//
//  tx_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef tx_proxy_py_h
#define tx_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>

void addTxProxyMethods(AllProxyClasses<blocksci::Transaction> &cls);

#endif /* tx_proxy_py_h */
