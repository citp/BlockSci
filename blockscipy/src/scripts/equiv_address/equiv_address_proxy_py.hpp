//
//  equiv_address_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef equiv_address_proxy_py_h
#define equiv_address_proxy_py_h

#include "python_fwd.hpp"

#include <blocksci/address/address_fwd.hpp>

void addEquivAddressProxyMethods(AllProxyClasses<blocksci::EquivAddress> &cls);

#endif /* equiv_address_proxy_py_h */
