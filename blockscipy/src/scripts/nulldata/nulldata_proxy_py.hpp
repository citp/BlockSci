//
//  nulldata_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef nulldata_proxy_py_h
#define nulldata_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addNulldataProxyMethodsMain(AllProxyClasses<blocksci::script::OpReturn, ProxyAddress> &cls);
void addNulldataProxyMethodsRange(AllProxyClasses<blocksci::script::OpReturn, ProxyAddress> &cls);

void addNulldataProxyMethods(AllProxyClasses<blocksci::script::OpReturn, ProxyAddress> &cls);

#endif /* nulldata_proxy_py_h */
