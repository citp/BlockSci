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

struct AddProxyMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T>> &cl) {
		cl
		.def("__call__", [](Proxy<T> &p, std::any &val) -> T {
			return p(val);
		})
		.def_property_readonly_static("range_proxy", [](pybind11::object &) -> Proxy<RawRange<T>> {
	        return {};
	    })
	    .def_property_readonly_static("iterator_proxy", [](pybind11::object &) -> Proxy<RawIterator<T>> {
	        return {};
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<ranges::optional<T>>, OptionalProxy> &cl) {
		cl
		.def("__call__", [](Proxy<ranges::optional<T>> &p, std::any &val) -> ranges::optional<T> {
			return p(val);
		})
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return {};
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<RawIterator<T>>, IteratorProxy, SequenceProxy<T>> &cl) {
		cl
		.def("__call__", [](Proxy<RawIterator<T>> &p, std::any &val) -> decltype(convertPythonRange(p.applySimple(val))) {
			return convertPythonRange(p.applySimple(val));
		})
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return {};
	    })
	    ;
	}

	template<typename T>
	void operator()(pybind11::class_<Proxy<RawRange<T>>, RangeProxy, SequenceProxy<T>> &cl) {
		cl
		.def("__call__", [](Proxy<RawRange<T>> &p, std::any &val) -> decltype(convertPythonRange(p.applySimple(val))) {
			return convertPythonRange(p.applySimple(val));
		})
		.def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<T> {
	        return {};
	    })
	    ;
	}
};

#endif /* proxy_basic_hpp */