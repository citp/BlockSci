//
//  proxy_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_py_hpp
#define proxy_py_hpp

#include "proxy.hpp"
#include "method_types.hpp"
#include "range_conversion.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <pybind11/pybind11.h>

#include <range/v3/view/transform.hpp>

#include <functional>

template<typename input_t, typename output_t>
Proxy<ranges::optional<input_t>, ranges::optional<output_t>> liftProxyOptional(Proxy<input_t, output_t> &p) {
	return [=](ranges::optional<input_t> &opt) -> ranges::optional<output_t> {
        if (opt) {
            return p(*opt);
        } else {
            return ranges::nullopt;
        }
	};
}

template<typename input_t, typename output_t>
Proxy<ranges::optional<input_t>, ranges::optional<output_t>> liftProxyOptional(Proxy<input_t, ranges::optional<output_t>> &p) {
	return [=](ranges::optional<input_t> &optional) -> ranges::optional<output_t> {
        if (optional) {
            return p(*optional);
        } else {
            return ranges::nullopt;
        }
	};
}

struct AddProxyMethods {
	template<typename input_t, typename output_t>
	void operator()(pybind11::class_<Proxy<input_t, output_t>> &cl) {
		using P = Proxy<input_t, output_t>;
		cl
		.def("__call__", [](P &p, input_t &input) -> output_t {
			return p(input);
		})
		// .def("__call__", [](P &p, ranges::any_view<input_t> &r) {
		// 	return convertRangeToPython(r | ranges::view::transform(p));
		// })
		// .def("__call__", [](P &p, ranges::any_view<input_t, ranges::category::random_access | ranges::category::sized> &r) {
		// 	return convertRangeToPython(r | ranges::view::transform(p));
		// })
		// .def("__call__", [](P &p, ranges::any_view<ranges::optional<input_t>> &r) {
		// 	return convertRangeToPython(r | ranges::view::transform(liftProxyOptional(p)));
		// })
		// .def("__call__", [](P &p, ranges::any_view<ranges::optional<input_t>, ranges::category::random_access | ranges::category::sized> &r) {
		// 	return convertRangeToPython(r | ranges::view::transform(liftProxyOptional(p)));
		// })
		;
	}
};

template <typename From, typename To>
std::string proxyName() {
	return PythonTypeName<From>::name() + "To" + PythonTypeName<To>::name() + "Proxy";
}

template<typename T>
using Iterator = ranges::any_view<T>;

template<typename T>
using Range = ranges::any_view<T, ranges::category::random_access | ranges::category::sized>;

template <typename T>
struct ProxyClasses {
	pybind11::class_<Proxy<blocksci::Block, T>> fromBlock;
    pybind11::class_<Proxy<blocksci::Transaction, T>> fromTx;
    pybind11::class_<Proxy<blocksci::Input, T>> fromInput;
    pybind11::class_<Proxy<blocksci::Output, T>> fromOutput;
    pybind11::class_<Proxy<blocksci::Address, T>> fromAddress;

    ProxyClasses(pybind11::module &m) : 
    fromBlock(m, strdup(proxyName<blocksci::Block, T>().c_str())),
    fromTx(m, strdup(proxyName<blocksci::Transaction, T>().c_str())),
    fromInput(m, strdup(proxyName<blocksci::Input, T>().c_str())),
    fromOutput(m, strdup(proxyName<blocksci::Output, T>().c_str())),
    fromAddress(m, strdup(proxyName<blocksci::Address, T>().c_str())) {}

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

    void setupBasicProxy() {
    	base.applyToAll(AddProxyMethods{});
    	optional.applyToAll(AddProxyMethods{});
    	iterator.applyToAll(AddProxyMethods{});
    	range.applyToAll(AddProxyMethods{});
    	optionalIterator.applyToAll(AddProxyMethods{});
    	optionalRange.applyToAll(AddProxyMethods{});
    }

    void setupSimplProxy() {
    	base.applyToAll(AddProxyMethods{});
    	optional.applyToAll(AddProxyMethods{});
    }
};

#endif /* proxy_py_hpp */


