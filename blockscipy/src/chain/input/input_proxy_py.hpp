//
//  input_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef input_proxy_py_h
#define input_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/chain/input.hpp>

void addInputProxyMethodsMain(AllProxyClasses<blocksci::Input> &cls);
void addInputProxyMethodsRange(AllProxyClasses<blocksci::Input> &cls);
void addInputProxyMethods(AllProxyClasses<blocksci::Input> &cls);

#endif /* input_proxy_py_h */
