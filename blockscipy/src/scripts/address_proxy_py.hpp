//
//  address_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef address_proxy_py_h
#define address_proxy_py_h

#include "proxy_py.hpp"
#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>

void addAddressProxyMethodsMain(AllProxyClasses<blocksci::AnyScript> &cls);
void addAddressProxyMethodsRange(AllProxyClasses<blocksci::AnyScript> &cls);
void addAddressProxyMethodsRangeMap(AllProxyClasses<blocksci::AnyScript> &cls);
void addAddressProxyMethodsRangeMapOptional(AllProxyClasses<blocksci::AnyScript> &cls);
void addAddressProxyMethodsRangeMapSequence(AllProxyClasses<blocksci::AnyScript> &cls);

void addAddressProxyMethods(AllProxyClasses<blocksci::AnyScript> &cls);

#endif /* address_proxy_py_h */
