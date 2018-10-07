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
#include "range_utils.hpp"
#include "python_range_conversion.hpp"

#include <blocksci/address/equiv_address.hpp>


struct CallFunc {
	template <typename T1, typename T2>
	static T2 apply(Proxy<T2> &p, T1 &val) {
		return p(val);
	}
};

struct IteratorCallFunc {
	template <typename T1, typename T2>
	static auto apply(Proxy<T2> &p, T1 &val) -> decltype(convertPythonRange(p(val))) {
		return convertPythonRange(p(val));
	}
};	

struct RangeCallFunc {
	template <typename T1, typename T2>
	static auto apply(Proxy<T2> &p, T1 &val) -> decltype(convertPythonRange(p(val))) {
		return convertPythonRange(p(val));
	}
};

template <typename R, typename T, typename Func>
void addCallMethod(pybind11::class_<Proxy<T>> &cl, Func) {
	cl
	.def("__call__", Func::template apply<R, T>)
	.def("__call__", Func::template apply<ranges::optional<R>, T>)
	.def("__call__", Func::template apply<Iterator<R>, T>)
	.def("__call__", Func::template apply<Range<R>, T>)
	;
}

template <typename T, typename Func>
void addCallMethods(pybind11::class_<Proxy<T>> &cl, Func callFunc) {
	using namespace blocksci;
	addCallMethod<Block>(cl, callFunc);
	addCallMethod<Transaction>(cl, callFunc);
	addCallMethod<Input>(cl, callFunc);
	addCallMethod<Output>(cl, callFunc);
	addCallMethod<AnyScript>(cl, callFunc);
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
}

struct AddProxyMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T>> &cl) {
		addCallMethods(cl, CallFunc{});
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<ranges::any_view<T>>> &cl) {
		addCallMethods(cl, IteratorCallFunc{});
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<ranges::any_view<T, random_access_sized>>> &cl) {
		addCallMethods(cl, RangeCallFunc{});
	}
};

#endif /* proxy_basic_hpp */