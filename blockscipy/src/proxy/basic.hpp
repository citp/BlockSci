//
//  basic.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_basic_hpp
#define proxy_basic_hpp

#include "proxy.hpp"
#include "caster_py.hpp"
#include "generic_sequence.hpp"
#include "python_range_conversion.hpp"

#include <blocksci/address/equiv_address.hpp>

#include <pybind11/chrono.h>

struct CallFunc {
	template <typename T1, typename T2>
	static typename T2::output_t apply(T2 &p, T1 &val) {
		return p(val);
	}
};

struct OptionalCallFunc {
	template <typename T1, typename T2>
	static typename T2::output_t apply(T2 &p, T1 &val) {
		return p(val);
	}
};

struct IteratorCallFunc {
	template <typename T1, typename T2>
	static auto apply(T2 &p, T1 &val) -> decltype(convertPythonRange(p(val))) {
		return convertPythonRange(p(val));
	}
};	

struct RangeCallFunc {
	template <typename T1, typename T2>
	static auto apply(T2 &p, T1 &val) -> decltype(convertPythonRange(p(val))) {
		return convertPythonRange(p(val));
	}
};

template <typename R, typename Class, typename Func>
void addCallMethod(Class &cl, Func) {
	using T = typename Class::type;
	cl
	.def("__call__", Func::template apply<R, T>)
	.def("__call__", Func::template apply<ranges::optional<R>, T>)
	.def("__call__", Func::template apply<Iterator<R>, T>)
	.def("__call__", Func::template apply<Range<R>, T>)
	;
}

template <typename T, typename Func>
void addCallMethods(T &cl, Func callFunc) {
	using namespace blocksci;
	addCallMethod<Block>(cl, callFunc);
	addCallMethod<Transaction>(cl, callFunc);
	addCallMethod<Input>(cl, callFunc);
	addCallMethod<Output>(cl, callFunc);
	addCallMethod<EquivAddress>(cl, callFunc);

	addCallMethod<script::Pubkey>(cl, callFunc);
	addCallMethod<script::PubkeyHash>(cl, callFunc);
	addCallMethod<script::WitnessPubkeyHash>(cl, callFunc);
	addCallMethod<script::MultisigPubkey>(cl, callFunc);
	addCallMethod<script::Multisig>(cl, callFunc);
	addCallMethod<script::ScriptHash>(cl, callFunc);
	addCallMethod<script::WitnessScriptHash>(cl, callFunc);
	addCallMethod<script::OpReturn>(cl, callFunc);
	addCallMethod<script::Nonstandard>(cl, callFunc);
	addCallMethod<script::WitnessUnknown>(cl, callFunc);

	addCallMethod<Cluster>(cl, callFunc);
	addCallMethod<TaggedCluster>(cl, callFunc);
	addCallMethod<TaggedAddress>(cl, callFunc);
}

struct AddProxyMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T>> &cl) {
		addCallMethods(cl, CallFunc{});

		cl
		.def_property_readonly_static("range_proxy", [](pybind11::object &) -> Proxy<Range<T>> {
	        return makeProxy<Range<T>>();
	    })
	    .def_property_readonly_static("iterator_proxy", [](pybind11::object &) -> Proxy<Iterator<T>> {
	        return makeProxy<Iterator<T>>();
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<ranges::optional<T>>, OptionalProxy> &cl) {
		addCallMethods(cl, OptionalCallFunc{});

		cl
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeProxy<T>();
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<Iterator<T>>, IteratorProxy> &cl) {
		addCallMethods(cl, IteratorCallFunc{});
		addCallMethod<blocksci::AnyScript>(cl, IteratorCallFunc{});

		cl
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeProxy<T>();
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<Range<T>>, RangeProxy> &cl) {
		addCallMethods(cl, RangeCallFunc{});
		addCallMethod<blocksci::AnyScript>(cl, RangeCallFunc{});

		cl
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeProxy<T>();
	    })
	    ;
	}
};

#endif /* proxy_basic_hpp */