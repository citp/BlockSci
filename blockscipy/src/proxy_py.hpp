//
//  proxy_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_py_hpp
#define proxy_py_hpp

#include "proxy.hpp"
#include "proxy_utils.hpp"
#include "range_utils.hpp"
#include "method_types.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <pybind11/pybind11.h>

#include <functional>

template <typename From, typename To>
std::string proxyName() {
	return PythonTypeName<From>::name() + "To" + PythonTypeName<To>::name() + "Proxy";
}

template <typename T1, typename T2>
void addProxyConditional(pybind11::module &m) {
    m
    .def("conditional", [](const Proxy<T1, bool> &cond, const Proxy<T1, T2> &p1, const Proxy<T1, T2> &p2) -> Proxy<T1, T2> {
        return std::function<T2(T1 &)>{[=](T1 &t) -> T2 {
            if(cond(t)) {
                return p1(t);
            } else {
                return p2(t);
            }
        }};
    })
    .def(("make" + PythonTypeName<T1>::name() + "Constant").c_str(), [](const T2 &val) -> Proxy<T1, T2> {
        return makeConstantProxy<T1>(val);
    })
    ;
}

template <typename T>
struct ProxyClasses {
	pybind11::class_<Proxy<blocksci::Block, T>> fromBlock;
    pybind11::class_<Proxy<blocksci::Transaction, T>> fromTx;
    pybind11::class_<Proxy<blocksci::Input, T>> fromInput;
    pybind11::class_<Proxy<blocksci::Output, T>> fromOutput;
    pybind11::class_<Proxy<blocksci::AnyScript, T>> fromAddress;

    ProxyClasses(pybind11::module &m) : 
    fromBlock(m, strdup(proxyName<blocksci::Block, T>().c_str())),
    fromTx(m, strdup(proxyName<blocksci::Transaction, T>().c_str())),
    fromInput(m, strdup(proxyName<blocksci::Input, T>().c_str())),
    fromOutput(m, strdup(proxyName<blocksci::Output, T>().c_str())),
    fromAddress(m, strdup(proxyName<blocksci::AnyScript, T>().c_str())) {

        addProxyConditional<blocksci::Block, T>(m);
        addProxyConditional<blocksci::Transaction, T>(m);
        addProxyConditional<blocksci::Input, T>(m);
        addProxyConditional<blocksci::Output, T>(m);
        addProxyConditional<blocksci::AnyScript, T>(m);
    }

    template<typename Func>
    void applyToAll(Func func) {
    	func(fromTx);
    	func(fromInput);
    	func(fromOutput);
    	func(fromAddress);
    }
};

template <typename T>
struct AllProxyClasses {
	ProxyClasses<T> base;
	ProxyClasses<ranges::optional<T>> optional;
	ProxyClasses<Iterator<T>> iterator;
	ProxyClasses<Range<T>> range;
	ProxyClasses<Iterator<ranges::optional<T>>> optionalIterator;
	ProxyClasses<Range<ranges::optional<T>>> optionalRange;

	AllProxyClasses(pybind11::module &m) : 
	base(m),
	optional(m),
	iterator(m),
	range(m),
	optionalIterator(m),
	optionalRange(m) {}

	template<typename Func>
    void applyToAll(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    	optionalIterator.applyToAll(func);
    	optionalRange.applyToAll(func);
    }

    template<typename Func>
    void applyToRanges(Func func) {
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    }

    template<typename Func>
    void applyToOptionalRanges(Func func) {
    	optionalIterator.applyToAll(func);
    	optionalRange.applyToAll(func);
    }

    template<typename Func>
    void setupBasicProxy(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    	optionalIterator.applyToAll(func);
    	optionalRange.applyToAll(func);
    }

    template<typename Func>
    void setupSimpleProxy(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    }
};

#endif /* proxy_py_hpp */


