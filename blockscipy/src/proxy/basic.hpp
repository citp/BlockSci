//
//  basic.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_basic_hpp
#define proxy_basic_hpp

#include "proxy.hpp"
#include "proxy_create.hpp"
#include "caster_py.hpp"
#include "generic_sequence.hpp"
#include "python_range_conversion.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

struct AddProxyMethods {
	template<typename T, typename BaseSimple>
	void operator()(pybind11::class_<Proxy<T>, BaseSimple> &cl) {
		cl
		.def("__call__", [](Proxy<T> &p, std::any &val) -> T {
			return p(val);
		})
		.def("__call__", [](Proxy<T> &p, GenericProxy &g) -> Proxy<T> {
			return std::function<T(std::any &)>{[generic = g.getGenericAny(), p](std::any &v) -> T {
				return p(generic(v));
			}};
		})
		.def_property_readonly_static("range_proxy", [](pybind11::object &) -> Proxy<RawRange<T>> {
	        return makeRangeProxy<T>();
	    })
	    .def_property_readonly_static("iterator_proxy", [](pybind11::object &) -> Proxy<RawIterator<T>> {
	        return makeIteratorProxy<T>();
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<ranges::optional<T>>, OptionalProxy> &cl) {
		cl
		.def("__call__", [](Proxy<ranges::optional<T>> &p, std::any &val) -> ranges::optional<T> {
			return p(val);
		})
		.def("__call__", [](Proxy<ranges::optional<T>> &p, GenericProxy &g) -> Proxy<ranges::optional<T>> {
			return std::function<ranges::optional<T>(std::any &)>{[generic = g.getGenericAny(), p](std::any &v) -> ranges::optional<T> {
				return p(generic(v));
			}};
		})
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeSimpleProxy<T>();
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<RawIterator<T>>, IteratorProxy, SequenceProxy<T>> &cl) {
		cl
		.def("__call__", [](Proxy<RawIterator<T>> &p, std::any &val) -> decltype(convertPythonRange(p.applySimple(val))) {
			return convertPythonRange(p.applySimple(val));
		})
		.def("__call__", [](Proxy<RawIterator<T>> &p, GenericProxy &g) -> Proxy<RawIterator<T>> {
			return std::function<RawIterator<T>(std::any &)>{[generic = g.getGenericAny(), p](std::any &v) -> RawIterator<T> {
				return p.applySimple(generic(v));
			}};
		})
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeSimpleProxy<T>();
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<RawRange<T>>, RangeProxy, SequenceProxy<T>> &cl) {
		cl
		.def("__call__", [](Proxy<RawRange<T>> &p, std::any &val) -> decltype(convertPythonRange(p.applySimple(val))) {
			return convertPythonRange(p.applySimple(val));
		})
		.def("__call__", [](Proxy<RawRange<T>> &p, GenericProxy &g) -> Proxy<RawRange<T>> {
			return std::function<RawRange<T>(std::any &)>{[generic = g.getGenericAny(), p](std::any &v) -> RawRange<T> {
				return p.applySimple(generic(v));
			}};
		})
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeSimpleProxy<T>();
	    })
	    ;
	}
};

#endif /* proxy_basic_hpp */