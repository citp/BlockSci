//
//  proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_hpp
#define proxy_hpp

#include "generic_sequence.hpp"

#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/transform.hpp>

#include <any>
#include <functional>

enum class ProxyType {
	Simple, Optional, Iterator, Range
};

template<typename T>
struct Proxy;

template<typename T>
Proxy<T> makeProxy() {
	return std::function<T(std::any &)>{[](std::any &t) -> T {
		return std::any_cast<T>(t);
	}};
}

template<typename T>
struct ProxyImpl {
	std::function<T(std::any &)> func;
	
	ProxyImpl(const std::function<T(std::any &)> &func_) : func(func_) {}

	ProxyImpl(const ProxyImpl<T> & proxy) : func(proxy.func) {}

	ProxyImpl(ProxyImpl<T> && proxy) : func(std::move(proxy.func)) {}

	ProxyImpl(const T &val) : ProxyImpl(std::function<T(std::any &)>{[=](std::any &) -> T {
		return val;
	}}) {}

	T operator()(std::any &t) const {
		return func(t);
	}

	T operator()(std::any && t) const {
		return func(t);
	}
};

struct GenericProxy {
	virtual ProxyType getProxyType() const = 0;
	virtual ~GenericProxy() = default;
};

struct SimpleProxy : public GenericProxy {
	virtual std::function<std::any(std::any &)> getGenericSimple() const = 0;
	virtual ~SimpleProxy() = default;

	ProxyType getProxyType() const override {
		return ProxyType::Simple;
	}
};

struct IteratorProxy : public GenericProxy {
	virtual std::function<Iterator<std::any>(std::any &)> getGenericIterator() const = 0;
	virtual ~IteratorProxy() = default;

	virtual ProxyType getProxyType() const override {
		return ProxyType::Iterator;
	}

	std::function<Iterator<std::any>(std::any &)> getGeneric() const {
		return getGenericIterator();
	}
};

struct RangeProxy : public IteratorProxy {
	virtual std::function<any_view<std::any, random_access_sized>(std::any &)> getGenericRange() const = 0;
	virtual ~RangeProxy() = default;

	ProxyType getProxyType() const override {
		return ProxyType::Range;
	}

	std::function<any_view<std::any, ranges::category::input>(std::any &)> getGenericIterator() const override {
		auto generic = getGenericRange();
		return [generic](std::any &val) -> Iterator<std::any> {
			return generic(val);
		};
	}

	std::function<any_view<std::any, random_access_sized>(std::any &)> getGeneric() const {
		return getGenericRange();
	}
};

struct OptionalProxy : public GenericProxy {
	virtual std::function<ranges::optional<std::any>(std::any &)> getGenericOptional() const = 0;
	virtual ~OptionalProxy() = default;

	ProxyType getProxyType() const override {
		return ProxyType::Optional;
	}

	std::function<ranges::optional<std::any>(std::any &)> getGeneric() const {
		return getGenericOptional();
	}
};

template <ranges::category range_cat>
struct SequenceProxyType;

template <>
struct SequenceProxyType<ranges::category::input> {
	using type = IteratorProxy;
};

template <>
struct SequenceProxyType<random_access_sized> {
	using type = RangeProxy;
};

template <ranges::category range_cat>
using proxy_sequence = typename SequenceProxyType<range_cat>::type;


struct ProxyAddress : public SimpleProxy {
	virtual std::function<blocksci::AnyScript(std::any &)> getGenericScript() const = 0;
	virtual ~ProxyAddress() = default;

	std::function<std::any(std::any &)> getGenericSimple() const {
		auto generic = getGenericScript();
		return [=](std::any &val) -> std::any {
			return generic(val);
		};
	}

	std::function<blocksci::AnyScript(std::any &)> getGeneric() const {
		return getGenericScript();
	}
};

template<typename T>
struct Proxy : public ProxyImpl<T>, public SimpleProxy {
	using output_t = T;
	using ProxyImpl<T>::ProxyImpl;

	std::function<std::any(std::any &)> getGenericSimple() const {
		return [f = this->func](std::any &val) -> std::any {
			return f(val);
		};
	}
};

template<typename T>
struct Proxy<ranges::optional<T>> : public ProxyImpl<ranges::optional<T>>, public OptionalProxy {
	using output_t = ranges::optional<T>;
	using ProxyImpl<ranges::optional<T>>::ProxyImpl;

	Proxy(const Proxy<T> &p) : Proxy(std::function<ranges::optional<T>(std::any &)>{[=](std::any &v) -> ranges::optional<T> {
		return p(v);
	}}) {}

	Proxy(const T &val) : Proxy(std::function<ranges::optional<T>(std::any &)>{[=](std::any &) -> ranges::optional<T> {
		return val;
	}}) {}

	std::function<ranges::optional<std::any>(std::any &)> getGenericOptional() const override {
		return [f = this->func](std::any &val) -> ranges::optional<std::any> {
			auto v = f(val);
			if (v) {
				return *v;
			} else {
				return ranges::nullopt;
			}
		};
	}
};

template<typename T>
struct SequenceProxy {
	virtual std::function<Iterator<T>(std::any &)> getIteratorFunc() const = 0;
	virtual ~SequenceProxy() = default;
};

template<typename T>
struct Proxy<Range<T>> : public ProxyImpl<Range<T>>, public SequenceProxy<T>, public RangeProxy {
	using output_t = Range<T>;
	using ProxyImpl<Range<T>>::ProxyImpl;

	std::function<Range<std::any>(std::any &)> getGenericRange() const override {
		return [f = this->func](std::any &val) -> Range<std::any> {
			return f(val).getAnySequence();
		};
	}

	std::function<Iterator<T>(std::any &)> getIteratorFunc() const override {
		return this->func;
	}
};

template<typename T>
struct Proxy<Iterator<T>> :  public ProxyImpl<Iterator<T>>, public SequenceProxy<T>, public IteratorProxy {
	using output_t = Iterator<T>;
	using ProxyImpl<Iterator<T>>::ProxyImpl;

	Proxy(const Proxy<Range<T>> &p) : Proxy(std::function<Iterator<T>(std::any &)>{
		[=](std::any &v) -> Iterator<T> {
		return p(v);
	}}) {}

	Proxy(Proxy<Range<T>> && p) : Proxy(std::function<Iterator<T>(std::any &)>{
		[=](std::any &v) -> Iterator<T> {
		return p(v);
	}}) {}

	std::function<Iterator<std::any>(std::any &)> getGenericIterator() const override {
		return [f = this->func](std::any &val) -> Iterator<std::any> {
			return f(val).getAnySequence();
		};
	}

	std::function<Iterator<T>(std::any &)> getIteratorFunc() const override {
		return [f = this->func](std::any &val) -> Iterator<T> {
			return f(val);
		};
	}
};

template<blocksci::AddressType::Enum type>
struct Proxy<blocksci::ScriptAddress<type>> : public ProxyImpl<blocksci::ScriptAddress<type>>, public ProxyAddress {
	using output_t = blocksci::ScriptAddress<type>;
	using ProxyImpl<blocksci::ScriptAddress<type>>::ProxyImpl;

	std::function<blocksci::AnyScript(std::any &)> getGenericScript() const override {
		return [f = this->func](std::any &val) -> blocksci::AnyScript {
			return blocksci::ScriptVariant{f(val)};
		};
	}
};

template<>
struct Proxy<blocksci::AnyScript> : public ProxyImpl<blocksci::AnyScript>, public ProxyAddress {
	using output_t = blocksci::AnyScript;
	using ProxyImpl<blocksci::AnyScript>::ProxyImpl;

	std::function<blocksci::AnyScript(std::any &)> getGenericScript() const override {
		return this->func;
	}
};

#endif /* proxy_hpp */
