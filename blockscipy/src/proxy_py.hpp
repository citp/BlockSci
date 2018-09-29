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

    pybind11::class_<Proxy<Iterator<blocksci::Block>, T>> fromBlockIterator;
    pybind11::class_<Proxy<Iterator<blocksci::Transaction>, T>> fromTxIterator;
    pybind11::class_<Proxy<Iterator<blocksci::Input>, T>> fromInputIterator;
    pybind11::class_<Proxy<Iterator<blocksci::Output>, T>> fromOutputIterator;
    pybind11::class_<Proxy<Iterator<blocksci::AnyScript>, T>> fromAddressIterator;

    // pybind11::class_<Proxy<Range<blocksci::Block>, T>> fromBlockRange;
    // pybind11::class_<Proxy<Range<blocksci::Transaction>, T>> fromTxRange;
    // pybind11::class_<Proxy<Range<blocksci::Input>, T>> fromInputRange;
    // pybind11::class_<Proxy<Range<blocksci::Output>, T>> fromOutputRange;
    // pybind11::class_<Proxy<Range<blocksci::AnyScript>, T>> fromAddressRange;

    ProxyClasses(pybind11::module &m) : 
    fromBlock(m, strdup(proxyName<blocksci::Block, T>().c_str()), pybind11::dynamic_attr()),
    fromTx(m, strdup(proxyName<blocksci::Transaction, T>().c_str()), pybind11::dynamic_attr()),
    fromInput(m, strdup(proxyName<blocksci::Input, T>().c_str()), pybind11::dynamic_attr()),
    fromOutput(m, strdup(proxyName<blocksci::Output, T>().c_str()), pybind11::dynamic_attr()),
    fromAddress(m, strdup(proxyName<blocksci::AnyScript, T>().c_str()), pybind11::dynamic_attr()),

    fromBlockIterator(m, strdup(proxyName<Iterator<blocksci::Block>, T>().c_str()), pybind11::dynamic_attr()),
    fromTxIterator(m, strdup(proxyName<Iterator<blocksci::Transaction>, T>().c_str()), pybind11::dynamic_attr()),
    fromInputIterator(m, strdup(proxyName<Iterator<blocksci::Input>, T>().c_str()), pybind11::dynamic_attr()),
    fromOutputIterator(m, strdup(proxyName<Iterator<blocksci::Output>, T>().c_str()), pybind11::dynamic_attr()),
    fromAddressIterator(m, strdup(proxyName<Iterator<blocksci::AnyScript>, T>().c_str()), pybind11::dynamic_attr()) {

    // fromBlockRange(m, strdup(proxyName<Range<blocksci::Block>, T>().c_str())),
    // fromTxRange(m, strdup(proxyName<Range<blocksci::Transaction>, T>().c_str())),
    // fromInputRange(m, strdup(proxyName<Range<blocksci::Input>, T>().c_str())),
    // fromOutputRange(m, strdup(proxyName<Range<blocksci::Output>, T>().c_str())),
    // fromAddressRange(m, strdup(proxyName<Range<blocksci::AnyScript>, T>().c_str())) {

        addProxyConditional<blocksci::Block, T>(m);
        addProxyConditional<blocksci::Transaction, T>(m);
        addProxyConditional<blocksci::Input, T>(m);
        addProxyConditional<blocksci::Output, T>(m);
        addProxyConditional<blocksci::AnyScript, T>(m);

        addProxyConditional<Iterator<blocksci::Block>, T>(m);
        addProxyConditional<Iterator<blocksci::Transaction>, T>(m);
        addProxyConditional<Iterator<blocksci::Input>, T>(m);
        addProxyConditional<Iterator<blocksci::Output>, T>(m);
        addProxyConditional<Iterator<blocksci::AnyScript>, T>(m);

        // addProxyConditional<Range<blocksci::Block>, T>(m);
        // addProxyConditional<Range<blocksci::Transaction>, T>(m);
        // addProxyConditional<Range<blocksci::Input>, T>(m);
        // addProxyConditional<Range<blocksci::Output>, T>(m);
        // addProxyConditional<Range<blocksci::AnyScript>, T>(m);
    }

    template<typename Func>
    void applyToAll(Func func) {
        func(fromBlock);
    	func(fromTx);
    	func(fromInput);
    	func(fromOutput);
    	func(fromAddress);

        func(fromBlockIterator);
        func(fromTxIterator);
        func(fromInputIterator);
        func(fromOutputIterator);
        func(fromAddressIterator);

        // func(fromBlockRange);
        // func(fromTxRange);
        // func(fromInputRange);
        // func(fromOutputRange);
        // func(fromAddressRange);
    }
};

template <typename T>
struct AllProxyClasses {
	ProxyClasses<T> base;
	ProxyClasses<ranges::optional<T>> optional;
	ProxyClasses<Iterator<T>> iterator;
	ProxyClasses<Range<T>> range;

	AllProxyClasses(pybind11::module &m) : 
	base(m),
	optional(m),
	iterator(m),
	range(m) {}

	template<typename Func>
    void applyToAll(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    }

    template<typename Func>
    void applyToRanges(Func func) {
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    }
    
    template<typename Func>
    void setupBasicProxy(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    	iterator.applyToAll(func);
    	range.applyToAll(func);
    }

    template<typename Func>
    void setupSimpleProxy(Func func) {
    	base.applyToAll(func);
    	optional.applyToAll(func);
    }
};

#endif /* proxy_py_hpp */


