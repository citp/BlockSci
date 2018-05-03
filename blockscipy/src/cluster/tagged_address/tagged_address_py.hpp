//
//  tagged_address_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 12/12/17.
//
//

#ifndef tagged_address_py_hpp
#define tagged_address_py_hpp

#include "method_tags.hpp"
#include "func_converter.hpp"

#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>

void init_tagged_address(pybind11::class_<blocksci::TaggedAddress> &cl);

struct AddTaggedAddressMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "address", toFunc([](const TaggedAddress &t) { return t.address; }), "Return the address object which has been tagged");
        func(property_tag, "tag", toFunc([](const TaggedAddress &t) { return t.tag; }), "Return the tag associated with the contained address");
    }
};


#endif /* tagged_address_py_hpp */
