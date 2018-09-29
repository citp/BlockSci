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


struct AddProxyMethods {
	template<typename input_t, typename output_t>
	void operator()(pybind11::class_<Proxy<input_t, output_t>> &cl) {
		using P = Proxy<input_t, output_t>;
		cl
		.def("__call__", [](P &p, input_t &input) {
			return p(input);
		})
		;
	}

	template<typename input_t, typename output_t>
	void operator()(pybind11::class_<Proxy<input_t, ranges::any_view<output_t>>> &cl) {
		using P = Proxy<input_t, ranges::any_view<output_t>>;
		cl
		.def("__call__", [](P &p, input_t &input) {
			return convertPythonRange(p(input));
		})
		;
	}

	template<typename input_t, typename output_t>
	void operator()(pybind11::class_<Proxy<input_t, ranges::any_view<output_t, random_access_sized>>> &cl) {
		using P = Proxy<input_t, ranges::any_view<output_t, random_access_sized>>;
		cl
		.def("__call__", [](P &p, input_t &input) {
			return convertPythonRange(p(input));
		})
		;
	}
};

#endif /* proxy_basic_hpp */