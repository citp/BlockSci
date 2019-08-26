//
//  block_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "block_proxy_py.hpp"
#include "block_properties_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"


struct AddBlockProxyMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        
        func(property_tag, "txes", +[](const Block &block) -> RawRange<Transaction> {
            return ranges::any_view<blocksci::Transaction, random_access_sized>{block};
        }, "A range of all of the txes in the block");
        func(property_tag, "inputs", +[](const Block &block) -> RawIterator<Input> {
            return ranges::any_view<blocksci::Input>{inputs(block)};
        }, "A range of all of the inputs in the block");
        func(property_tag, "outputs", +[](const Block &block) -> RawIterator<Output> {
            return ranges::any_view<blocksci::Output>{outputs(block)};
        }, "A range of all of the outputs in the block");
        ;
    }
};

void addBlockProxyMethods(AllProxyClasses<blocksci::Block> &cls) {
    cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
	addProxyOptionalMethods(cls.optional);

    applyMethodsToProxy(cls.base, AddBlockMethods{});
    applyMethodsToProxy(cls.base, AddBlockProxyMethods{});
    addProxyEqualityMethods(cls.base);
}
