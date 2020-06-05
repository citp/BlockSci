//
//  scripthash_proxy_impl.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_scripthash_proxy_impl_h
#define blocksci_scripthash_proxy_impl_h

#include "python_fwd.hpp"
#include "method_tags.hpp"

#include <pybind11/pybind11.h>

template <typename T>
struct AddScriptHashBaseMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(property_tag, "wrapped_address", &T::getWrappedAddress, "The address inside this P2SH address");
        func(property_tag, "raw_address",  &T::getAddressHash, "The 160 bit P2SH address hash");
        func(property_tag, "address_string", &T::addressString, "Bitcoin address string");
    }
};

#endif /* blocksci_scripthash_proxy_impl_h */
