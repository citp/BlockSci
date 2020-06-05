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
#include "method_types.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

struct AddProxyMethods {
	template<typename T, typename BaseSimple>
	void operator()(pybind11::class_<Proxy<T>, BaseSimple> &cl) {
		cl
		.def("__call__", [](Proxy<T> &p, std::any &val) -> T {
			return p(val);
		})
		.def("__call__", compose<T>)
		.def_property_readonly_static("output_type_name", [](pybind11::object &) {
	        return typenameLookup().getDocstringName(typeid(T));
	    })
	    .def_property_readonly_static("optional_proxy", [](pybind11::object &) -> Proxy<ranges::optional<T>> {
	        return makeOptionalProxy<T>();
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
		.def("__call__", compose<ranges::optional<T>>)
		.def_property_readonly_static("output_type_name", [](pybind11::object &) {
	        return typenameLookup().getDocstringName(typeid(ranges::optional<T>));
	    })
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeSimpleProxy<T>();
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<RawIterator<T>>, IteratorProxy, SequenceProxy<T>> &cl) {
		using return_type = decltype(convertPythonRange(std::declval<typename Proxy<RawIterator<T>>::output_t>()));
		cl
		.def("__call__", [](Proxy<RawIterator<T>> &p, std::any &val) -> return_type {
			return convertPythonRange(p(val));
		})
		.def("__call__", compose<RawIterator<T>>)
		.def_property_readonly_static("output_type_name", [](pybind11::object &) {
	        return typenameLookup().getDocstringName(typeid(return_type));
	    })
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeSimpleProxy<T>();
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<RawRange<T>>, RangeProxy, SequenceProxy<T>> &cl) {
		using return_type = decltype(convertPythonRange(std::declval<typename Proxy<RawRange<T>>::output_t>()));
		cl
		.def("__call__", [](Proxy<RawRange<T>> &p, std::any &val) -> return_type {
			return convertPythonRange(p(val));
		})
		.def("__call__", compose<RawRange<T>>)
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return makeSimpleProxy<T>();
	    })
	    .def_property_readonly_static("output_type_name", [](pybind11::object &) {
	        return typenameLookup().getDocstringName(typeid(return_type));
	    })
	    ;
	}
};

#endif /* proxy_basic_hpp */