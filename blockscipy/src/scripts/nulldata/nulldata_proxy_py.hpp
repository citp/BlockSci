//
//  nulldata_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef nulldata_proxy_py_h
#define nulldata_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/nulldata_script.hpp>

void addNulldataProxyMethodsMain(AllProxyClasses<blocksci::script::OpReturn> &cls);
void addNulldataProxyMethodsRange(AllProxyClasses<blocksci::script::OpReturn> &cls);

void addNulldataProxyMethods(AllProxyClasses<blocksci::script::OpReturn> &cls);

#endif /* nulldata_proxy_py_h */
