//
//  tagged_address_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "tagged_address_proxy_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

struct AddTaggedAddressMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(property_tag, "address", +[](const TaggedAddress &t) { return t.address; }, "Return the address object which has been tagged");
        func(property_tag, "tag", +[](const TaggedAddress &t) { return t.tag; }, "Return the tag associated with the contained address");
    }
};

void addTaggedAddressProxyMethods(AllProxyClasses<blocksci::TaggedAddress> &cls) {
	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

    applyMethodsToProxy(cls.base, AddTaggedAddressMethods{});
    addProxyEqualityMethods(cls.base);
}
