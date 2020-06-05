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

template <typename T, typename BaseSimple>
void setupRangesProxy(AllProxyClasses<T, BaseSimple> &cls) {
	cls.sequence
	.def("_where", [](SequenceProxy<T> &p, Proxy<bool> &p2) -> Proxy<RawIterator<T>> {
		return liftSequence(p, [p2](auto && seq) -> RawIterator<T> {
			return ranges::views::filter(std::forward<decltype(seq)>(seq), [p2](T item) {
				return p2(std::move(item));
			});
		});
	})
	.def("_max", [](SequenceProxy<T> &p, Proxy<int64_t> &p2) -> Proxy<ranges::optional<T>> {
		// Copied and modified from range/v3/algorithm/max.hpp
		// Testing whether input range was empty required modification
		return liftSequence(p, [p2](auto && rng) -> ranges::optional<T> {
			auto begin = ranges::begin(rng);
            auto end = ranges::end(rng);
            if (begin == end) {
            	return ranges::nullopt;
            }

            T result = *begin;
            while(++begin != end) {
                auto && tmp = *begin;
                if (p2(result) < p2(tmp)) {
                    result = (decltype(tmp) &&) tmp;
                }
            }
            return result;
		});
	})
	.def("_min", [](SequenceProxy<T> &p, Proxy<int64_t> &p2) -> Proxy<ranges::optional<T>> {
		// Copied and modified from range/v3/algorithm/min.hpp
		// Testing whether input range was empty required modification
		return liftSequence(p, [p2](auto && rng) -> ranges::optional<T> {
			auto begin = ranges::begin(rng);
            auto end = ranges::end(rng);
            if (begin == end) {
            	return ranges::nullopt;
            }

            T result = *begin;
            while(++begin != end) {
                auto && tmp = *begin;
                if (p2(result) > p2(tmp)) {
                    result = (decltype(tmp) &&) tmp;
                }
            }
            return result;
		});
	})
	;

	cls.range
    .def("__getitem__", [](Proxy<RawRange<T>> &p, int64_t posIndex) -> Proxy<T> {
    	return lift(p, [posIndex](auto && range) -> T {
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
    .def("__getitem__", [](Proxy<RawRange<T>> &p, pybind11::slice slice) -> Proxy<RawRange<T>> {
    	return lift(p, [slice](auto && range) -> RawRange<T> {
    		size_t start, stop, step, slicelength;
	        auto chainSize = ranges::size(range);
	        if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
	            throw pybind11::error_already_set();
	        
	        auto subset =  range[{static_cast<ranges::range_size_type_t<RawRange<T>>>(start), static_cast<ranges::range_size_type_t<RawRange<T>>>(stop)}];
	        return subset | ranges::views::stride(step);
    	});
    }, pybind11::arg("slice"))
	;
}


#endif /* proxy_range_hpp */
