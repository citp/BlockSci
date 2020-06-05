//
//  proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_hpp
#define proxy_hpp

#include "generic_proxy.hpp"
#include "proxy_type_check.hpp"
#include "blocksci_type.hpp"
#include "blocksci_iterator_type.hpp"
#include "blocksci_range_type.hpp"

#include <range/v3/view/empty.hpp>
#include <range/v3/view/single.hpp>

template<typename T>
struct SequenceProxy {
	virtual std::function<RawIterator<T>(std::any &)> getIteratorFunc() const = 0;
	virtual ~SequenceProxy() = default;

	virtual ProxyTypeInfo getSourceType() const = 0;
	virtual ProxyTypeInfo getDestType() const = 0;
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


template<typename T>
struct Proxy : public SimpleProxy {
	using output_t = T;
	
	std::function<output_t(std::any &)> func;
	ProxyTypeInfo sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const ProxyTypeInfo &sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<BlocksciType(std::any &)> getGenericSimple() const override {
		return [f = this->func](std::any &val) -> BlocksciType {
			return BlocksciType{f(val)};
		};
	}

	ProxyTypeInfo getSourceType() const override {
		return sourceType;
	}

	ProxyTypeInfo getDestType() const override {
		return createProxyTypeInfo<output_t>();
	}
};

template<typename T>
struct Proxy<ranges::optional<T>> : public OptionalProxy {
	using output_t = ranges::optional<T>;
	
	std::function<output_t(std::any &)> func;
	ProxyTypeInfo sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const ProxyTypeInfo &sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<ranges::optional<BlocksciType>(std::any &)> getGenericOptional() const override {
		return [f = this->func](std::any &val) -> ranges::optional<BlocksciType> {
			auto v = f(val);
			if (v) {
				return BlocksciType{*v};
			} else {
				return ranges::nullopt;
			}
		};
	}

	std::function<BlocksciRangeType(std::any &)> getGenericRange() const override {
		return [f = this->func](std::any &val) -> BlocksciRangeType {
			auto v = f(val);
			if (v) {
				return BlocksciRangeType{RawRange<T>{ranges::views::single(*v)}};
			} else {
				return BlocksciRangeType{RawRange<T>{ranges::views::empty<T>}};
			}
		};
	}

	std::function<std::any(std::any &)> getGenericAny() const override {
		return [f = this->func](std::any &val) -> std::any {
			return f(val);
		};
	}

	ProxyTypeInfo getSourceType() const override {
		return sourceType;
	}

	ProxyTypeInfo getDestType() const override {
		return createProxyTypeInfo<output_t>();
	}
};

template<typename T>
struct Proxy<RawRange<T>> : public SequenceProxy<T>, public RangeProxy {
	using output_t = RawRange<T>;
	
	std::function<output_t(std::any &)> func;
	ProxyTypeInfo sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const ProxyTypeInfo &sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<BlocksciRangeType(std::any &)> getGenericRange() const override {
		return [f = this->func](std::any &val) -> BlocksciRangeType {
			return BlocksciRangeType{f(val)};
		};
	}

	std::function<RawIterator<T>(std::any &)> getIteratorFunc() const override {
		return this->func;
	}

	ProxyTypeInfo getSourceType() const override {
		return sourceType;
	}

	ProxyTypeInfo getDestType() const override {
		return createProxyTypeInfo<output_t>();
	}
};

template<typename T>
struct Proxy<RawIterator<T>> : public SequenceProxy<T>, public IteratorProxy {
	using output_t = RawIterator<T>;
	
	std::function<output_t(std::any &)> func;
	ProxyTypeInfo sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const ProxyTypeInfo &sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<BlocksciIteratorType(std::any &)> getGenericIterator() const override {
		return [f = this->func](std::any &val) -> BlocksciIteratorType {
			return BlocksciIteratorType{f(val)};
		};
	}

	std::function<RawIterator<T>(std::any &)> getIteratorFunc() const override {
		return this->func;
	}

	ProxyTypeInfo getSourceType() const override {
		return sourceType;
	}

	ProxyTypeInfo getDestType() const override {
		return createProxyTypeInfo<output_t>();
	}
};

template<blocksci::AddressType::Enum type>
struct Proxy<blocksci::ScriptAddress<type>> : public ProxyAddress {
	using output_t = blocksci::ScriptAddress<type>;
	
	std::function<output_t(std::any &)> func;
	ProxyTypeInfo sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const ProxyTypeInfo &sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<blocksci::AnyScript(std::any &)> getGenericScript() const override {
		return [f = this->func](std::any &val) -> blocksci::AnyScript {
			return blocksci::ScriptVariant{f(val)};
		};
	}

	ProxyTypeInfo getSourceType() const override {
		return sourceType;
	}

	ProxyTypeInfo getDestType() const override {
		return createProxyTypeInfo<output_t>();
	}
};

template<>
struct Proxy<blocksci::AnyScript> : public ProxyAddress {
	using output_t = blocksci::AnyScript;
	
	std::function<output_t(std::any &)> func;
	ProxyTypeInfo sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const ProxyTypeInfo &sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<blocksci::AnyScript(std::any &)> getGenericScript() const override {
		return this->func;
	}

	ProxyTypeInfo getSourceType() const override {
		return sourceType;
	}

	ProxyTypeInfo getDestType() const override {
		return createProxyTypeInfo<output_t>();
	}
};

#endif /* proxy_hpp */
