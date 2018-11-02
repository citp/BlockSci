//
//  range.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_hpp
#define proxy_range_hpp

#include "proxy_py.hpp"
#include "proxy_utils.hpp"

#include <range/v3/view/filter.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/size.hpp>

template<typename T, ranges::category range_cat, typename... Args>
void addProxySequenceMethods(pybind11::class_<Proxy<any_view<T, range_cat>>, Args...> &cl) {
	cl
	.def("_where", [](Proxy<any_view<T, range_cat>> &p, Proxy<bool> &p2) -> Proxy<Iterator<T>> {
		return lift(p, [=](any_view<T, range_cat> && range) -> Iterator<T> {
			return range | ranges::view::filter([=](T item) {
				return p2(std::move(item));
			});
		});
	})
	;
}

template<typename T>
void addRangeProxyMethods(pybind11::class_<Proxy<Range<T>>, RangeProxy> &cl) {
	cl
    .def("__getitem__", [](Proxy<Range<T>> &p, int64_t posIndex) -> Proxy<T> {
    	return lift(p, [=](Range<T> && range) -> T {
			auto chainSize = static_cast<int64_t>(range.size());
			auto pos = posIndex;
	        if (pos < 0) {
	            pos += chainSize;
	        }
	        if (pos < 0 || pos >= chainSize) {
	            throw pybind11::index_error();
	        }
	        return range[pos];
		});
    }, pybind11::arg("index"))
    .def("__getitem__", [](Proxy<Range<T>> &p, pybind11::slice slice) -> Proxy<Range<T>> {
    	return lift(p, [=](Range<T> && range) -> Range<T> {
	        size_t start, stop, step, slicelength;
	        const auto &constRange = range;
	        auto chainSize = ranges::size(constRange);
	        if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
	            throw pybind11::error_already_set();
	        
	        auto subset =  range[{static_cast<ranges::range_size_type_t<Range<T>>>(start), static_cast<ranges::range_size_type_t<Range<T>>>(stop)}];
	        return subset | ranges::view::stride(step);
	    });
    }, pybind11::arg("slice"))
	;

}

template <typename T>
void setupRangesProxy(AllProxyClasses<T> &cls) {
	addProxySequenceMethods<T, ranges::category::input>(cls.iterator);
	addProxySequenceMethods<T, random_access_sized>(cls.range);
	addRangeProxyMethods(cls.range);
}


#endif /* proxy_range_hpp */
