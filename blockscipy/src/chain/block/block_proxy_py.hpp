//
//  block_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef block_proxy_py_h
#define block_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>

void addBlockProxyMethods(AllProxyClasses<blocksci::Block> &cls);

#endif /* block_proxy_py_h */
