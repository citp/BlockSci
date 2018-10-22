//
//  witness_unknown_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/22/18.
//

#ifndef witness_unknown_proxy_py_h
#define witness_unknown_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/scripts/witness_unknown_script.hpp>

void addWitnessUnknownProxyMethods(AllProxyClasses<blocksci::script::WitnessUnknown> &cls);

#endif /* witness_unknown_proxy_py_h */
