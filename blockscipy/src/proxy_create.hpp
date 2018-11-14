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
	return {std::function<T(std::any &)>{[](std::any &t) -> T {
		return std::any_cast<T>(t);
	}}, &typeid(T)};
}

template<typename T>
Proxy<ranges::optional<T>> makeOptionalProxy() {
	return {std::function<ranges::optional<T>(std::any &)>{[](std::any &t) -> ranges::optional<T> {
		return std::any_cast<ranges::optional<T>>(t);
	}}, &typeid(ranges::optional<T>)};
}

template<typename T>
Proxy<RawIterator<T>> makeIteratorProxy() {
	return {std::function<RawIterator<T>(std::any &)>{[](std::any &t) -> RawIterator<T> {
		RawIterator<BlocksciType> *rawIt = std::any_cast<RawIterator<BlocksciType>>(&t);
		if (rawIt != nullptr) {
			return ranges::view::transform(*rawIt, [](BlocksciType && r) -> T { return mpark::get<T>(r.var); });
		}
		return std::any_cast<RawIterator<T>>(t);
	}}, &typeid(RawIterator<T>)};
}

template<typename T>
Proxy<RawRange<T>> makeRangeProxy() {
	return {std::function<RawRange<T>(std::any &)>{[](std::any &t) -> RawRange<T> {
		RawRange<BlocksciType> *rawIt = std::any_cast<RawRange<BlocksciType>>(&t);
		if (rawIt != nullptr) {
			return ranges::view::transform(*rawIt, [](BlocksciType && r) -> T { return mpark::get<T>(r.var); });
		}
		return std::any_cast<RawRange<T>>(t);
	}}, &typeid(RawRange<T>)};
}

#endif /* proxy_create_hpp */
