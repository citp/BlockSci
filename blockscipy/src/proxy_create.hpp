//
//  proxy_create.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/11/18.
//

#ifndef proxy_create_hpp
#define proxy_create_hpp

#include "proxy.hpp"


template<typename T>
Proxy<T> makeSimpleProxy() {
	return std::function<T(std::any &)>{[](std::any &t) -> T {
		return std::any_cast<T>(t);
	}};
}

template<typename T>
Proxy<ranges::optional<T>> makeOptionalProxy() {
	return std::function<ranges::optional<T>(std::any &)>{[](std::any &t) -> ranges::optional<T> {
		return std::any_cast<ranges::optional<T>>(t);
	}};
}

template<typename T>
Proxy<RawIterator<T>> makeIteratorProxy() {
	return std::function<RawIterator<T>(std::any &)>{[](std::any &t) -> RawIterator<T> {
		return std::any_cast<RawIterator<T>>(t);
	}};
}

template<typename T>
Proxy<RawRange<T>> makeRangeProxy() {
	return std::function<RawRange<T>(std::any &)>{[](std::any &t) -> RawRange<T> {
		return std::any_cast<RawRange<T>>(t);
	}};
}

#endif /* proxy_create_hpp */
