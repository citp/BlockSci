//
//  output_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef output_proxy_py_h
#define output_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/chain/output.hpp>

void addOutputProxyMethods(AllProxyClasses<blocksci::Output> &cls);

#endif /* output_proxy_py_h */
