//
//  range_map.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_sequence_hpp
#define proxy_range_map_sequence_hpp

#include "proxy_py.hpp"
#include "proxy_utils.hpp"

#include <blocksci/address/equiv_address.hpp>

#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>


template<typename T, typename R>
Proxy<Iterator<R>> mapIteratorProxy(Proxy<T> &p1, Proxy<Iterator<R>> &p2) {
	return lift(p1, [=](T && val) -> Iterator<R> {
		return ranges::view::join(ranges::view::transform(std::move(val), p2));
	});
}

template<typename T, typename R>
Proxy<Iterator<R>> mapRangeProxy(Proxy<T> &p1, Proxy<Range<R>> &p2) {
	return lift(p1, [=](T && val) -> Iterator<R> {
		return ranges::view::join(ranges::view::transform(std::move(val), p2));
	});
}

template<typename R, typename T>
void addProxyMapSequenceFunc(pybind11::class_<Proxy<T>> &cl) {
    cl
    .def("map", mapIteratorProxy<T, R>)
    .def("map", mapRangeProxy<T, R>)
    ;
}


struct AddProxyMapSequenceFuncsMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T>> &cl) {
		using namespace blocksci;
		addProxyMapSequenceFunc<Block>(cl);
		addProxyMapSequenceFunc<Transaction>(cl);
		addProxyMapSequenceFunc<Input>(cl);
		addProxyMapSequenceFunc<Output>(cl);
		addProxyMapSequenceFunc<AnyScript>(cl);
		addProxyMapSequenceFunc<EquivAddress>(cl);
	}
};

template <typename T>
void setupRangesMapSequenceProxy(AllProxyClasses<T> &cls) {
	cls.iterator.applyToAll(AddProxyMapSequenceFuncsMethods{});
	cls.range.applyToAll(AddProxyMapSequenceFuncsMethods{});
}


#endif /* proxy_range_map_sequence_hpp */
