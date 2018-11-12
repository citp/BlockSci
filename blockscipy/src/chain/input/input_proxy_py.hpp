//
//  input_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef input_proxy_py_h
#define input_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/chain/chain_fwd.hpp>

void addInputProxyMethods(AllProxyClasses<blocksci::Input> &cls);

#endif /* input_proxy_py_h */
