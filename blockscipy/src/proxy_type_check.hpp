//
//  proxy_type_check.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/14/18.
//
//

#ifndef blocksci_proxy_type_check_hpp
#define blocksci_proxy_type_check_hpp

#include "python_fwd.hpp"
#include <range/v3/utility/optional.hpp>

#include <typeinfo>

struct ProxyTypeInfo {
	const std::type_info *type;
	const std::type_info *baseType;
	ProxyType kind;

	void checkMatch(const ProxyTypeInfo &other) const;
	void checkAccept(const ProxyTypeInfo &other) const;
};

template <typename T>
struct ProxyTypeInfoCreator {
	ProxyTypeInfo operator()() const {
		return {&typeid(T), &typeid(T), ProxyType::Simple};
	}
};

template <typename T>
struct ProxyTypeInfoCreator<ranges::optional<T>> {
	ProxyTypeInfo operator()() const {
		return {&typeid(ranges::optional<T>), &typeid(T), ProxyType::Optional};
	}
};

template <typename T>
struct ProxyTypeInfoCreator<RawIterator<T>> {
	ProxyTypeInfo operator()() const {
		return {&typeid(RawIterator<T>), &typeid(T), ProxyType::Iterator};
	}
};

template <typename T>
struct ProxyTypeInfoCreator<RawRange<T>> {
	ProxyTypeInfo operator()() const {
		return {&typeid(RawRange<T>), &typeid(T), ProxyType::Range};
	}
};

template <typename T>
inline ProxyTypeInfo createProxyTypeInfo() {
	return ProxyTypeInfoCreator<T>{}();
}

#endif /* blocksci_proxy_type_check_hpp */
