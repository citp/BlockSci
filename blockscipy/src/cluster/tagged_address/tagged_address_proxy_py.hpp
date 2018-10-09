//
//  tagged_address_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef tagged_address_proxy_py_h
#define tagged_address_proxy_py_h

#include "proxy_py.hpp"

#include <blocksci/cluster/cluster.hpp>

void addTaggedAddressProxyMethods(AllProxyClasses<blocksci::TaggedAddress> &cls);

#endif /* block_proxy_py_h */
