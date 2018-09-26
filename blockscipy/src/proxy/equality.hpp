//
//  equality.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_equality_hpp
#define proxy_equality_hpp

struct AddProxyEqualityMethods {
	template<typename Class>
	void operator()(Class &cl) {
		using P = typename Class::type;
		cl
		.def("__eq__", [](P &p, typename P::output_t &output) {
			return p == output;
		})
		.def("__ne__", [](P &p, typename P::output_t &output) {
			return p != output;
		})
		.def("__eq__", [](P &p1, P &p2) {
			return p1 == p2;
		})
		.def("__ne__", [](P &p1, P &p2) {
			return p1 != p2;
		})
		;
	}
};

#endif /* proxy_equality_hpp */