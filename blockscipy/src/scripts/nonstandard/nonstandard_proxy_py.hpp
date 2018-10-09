//
//  nonstandard_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef nonstandard_proxy_py_h
#define nonstandard_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/nonstandard_script.hpp>

void addNonstandardProxyMethods(AllProxyClasses<blocksci::script::Nonstandard> &cls);

#endif /* nonstandard_proxy_py_h */
