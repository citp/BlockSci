//
//  witness_unknown_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/22/18.
//

#ifndef witness_unknown_proxy_py_h
#define witness_unknown_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addWitnessUnknownProxyMethods(AllProxyClasses<blocksci::script::WitnessUnknown> &cls);

#endif /* witness_unknown_proxy_py_h */
