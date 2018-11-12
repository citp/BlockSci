//
//  optional.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/1/18.
//


#ifndef generic_proxy_optional_hpp
#define generic_proxy_optional_hpp

#include "python_fwd.hpp"

#include <pybind11/pybind11.h>

void addOptionalProxyMethods(pybind11::class_<OptionalProxy, GenericProxy> &cl);

#endif /* generic_proxy_optional_hpp */
