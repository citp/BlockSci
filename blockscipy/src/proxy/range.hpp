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

template <typename T>
void setupRangesProxy(AllProxyClasses<T> &cls) {
	cls.sequence
	.def("_where", [](SequenceProxy<T> &seq, Proxy<bool> &p2) -> Proxy<Iterator<T>> {
		auto generic = seq.getIteratorFunc();
		return std::function<Iterator<T>(std::any &)>{[=](std::any &val) -> Iterator<T> {
			auto rng = generic(val);
			return ranges::any_view<T>{ranges::view::filter(rng, [=](T item) {
				return p2(std::move(item));
			})};
		}};
	})
	;

	cls.range
    .def("__getitem__", [](Proxy<Range<T>> &p, int64_t posIndex) -> Proxy<T> {
    	return lift(p, [=](Range<T> && range) -> T {
			auto chainSize = static_cast<int64_t>(range.rng.size());
			auto pos = posIndex;
	        if (pos < 0) {
	            pos += chainSize;
	        }
	        if (pos < 0 || pos >= chainSize) {
	            throw pybind11::index_error();
	        }
	        return range.rng[pos];
		});
    }, pybind11::arg("index"))
    .def("__getitem__", [](Proxy<Range<T>> &p, pybind11::slice slice) -> Proxy<Range<T>> {
    	return lift(p, [=](Range<T> && range) -> Range<T> {
	        size_t start, stop, step, slicelength;
	        const auto &constRange = range;
	        auto chainSize = ranges::size(constRange.rng);
	        if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
	            throw pybind11::error_already_set();
	        
	        auto subset =  range.rng[{static_cast<ranges::range_size_type_t<Range<T>>>(start), static_cast<ranges::range_size_type_t<Range<T>>>(stop)}];
	        return ranges::any_view<T, random_access_sized>{subset | ranges::view::stride(step)};
	    });
    }, pybind11::arg("slice"))
	;
}


#endif /* proxy_range_hpp */
