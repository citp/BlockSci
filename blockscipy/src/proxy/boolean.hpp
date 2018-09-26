//
//  boolean.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_boolean_hpp
#define proxy_boolean_hpp


struct AddProxyBooleanMethods {
	template<typename T>
	void operator()(pybind11::class_<Proxy<T, bool>> &cl) {
		using P = Proxy<T, bool>;
		cl
		.def("__and__", [](P &p, bool val) {
			return p && val;
		})
		.def("__or__", [](P &p, bool val) {
			return p || val;
		})
		.def("__and__", [](P &p1, P &p2) {
			return p1 && p2;
		})
		.def("__or__", [](P &p1, P &p2) {
			return p1 || p2;
		})
		.def("__invert__", [](P &p) -> Proxy<T, bool> {
			return std::function<bool(T &)>{[=](T &t) {
				return !p(t);
			}};
		})
		;
	}
};

#endif /* proxy_boolean_hpp */