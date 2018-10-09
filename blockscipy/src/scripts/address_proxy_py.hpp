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

#include <blocksci/scripts/script_variant.hpp>

void init_proxy_address(pybind11::class_<ProxyAddress> &addressCl);

void addAddressProxyMethods(AllProxyClasses<blocksci::AnyScript> &cls);

#endif /* address_proxy_py_h */
