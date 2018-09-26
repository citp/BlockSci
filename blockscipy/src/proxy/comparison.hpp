//
//  comparison.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_comparison_hpp
#define proxy_comparison_hpp

struct AddProxyComparisonMethods {
	template<typename Class>
	void operator()(Class &cl) {
		using P = typename Class::type;
		cl
		.def("__lt__", [](P &p, typename P::output_t &output) {
			return p < output;
		})
		.def("__le__", [](P &p, typename P::output_t &output) {
			return p <= output;
		})
		.def("__gt__", [](P &p, typename P::output_t &output) {
			return p > output;
		})
		.def("__ge__", [](P &p, typename P::output_t &output) {
			return p >= output;
		})
		.def("__lt__", [](P &p1, P &p2) {
			return p1 < p2;
		})
		.def("__le__", [](P &p1, P &p2) {
			return p1 <= p2;
		})
		.def("__gt__", [](P &p1, P &p2) {
			return p1 > p2;
		})
		.def("__ge__", [](P &p1, P &p2) {
			return p1 >= p2;
		})
		;
	}
};

#endif /* proxy_comparison_hpp */