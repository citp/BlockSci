//
//  nonstandard_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef nonstandard_proxy_py_h
#define nonstandard_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/chain/input.hpp>

void addNonstandardProxyMethodsMain(AllProxyClasses<blocksci::script::Nonstandard> &cls);
void addNonstandardProxyMethodsRange(AllProxyClasses<blocksci::script::Nonstandard> &cls);
void addNonstandardProxyMethodsRangeMap(AllProxyClasses<blocksci::script::Nonstandard> &cls);
void addNonstandardProxyMethodsRangeMapOptional(AllProxyClasses<blocksci::script::Nonstandard> &cls);
void addNonstandardProxyMethodsRangeMapSequence(AllProxyClasses<blocksci::script::Nonstandard> &cls);

void addNonstandardProxyMethods(AllProxyClasses<blocksci::script::Nonstandard> &cls);

#endif /* nonstandard_proxy_py_h */
