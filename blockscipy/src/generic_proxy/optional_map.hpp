//
//  optional_map.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 10/31/18.
//


#ifndef proxy_optional_map_hpp
#define proxy_optional_map_hpp

#include "python_fwd.hpp"

#include <pybind11/pybind11.h>

void addOptionalProxyMapMethods(pybind11::class_<OptionalProxy, GenericProxy> &cl);

#endif /* proxy_optional_map_hpp */
