//
//  proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_hpp
#define proxy_hpp

#include "generic_proxy.hpp"
#include "python_range_conversion.hpp"

#include <range/v3/view/transform.hpp>

template<typename T>
struct SequenceProxy {
	virtual std::function<RawIterator<T>(std::any &)> getIteratorFunc() const = 0;
	virtual ~SequenceProxy() = default;

	virtual const std::type_info *getSourceType() const = 0;
	virtual const std::type_info *getDestType() const = 0;
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
	const std::type_info *sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const std::type_info *sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

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

	const std::type_info *getSourceType() const override {
		return sourceType;
	}

	const std::type_info *getDestType() const override {
		return &typeid(output_t);
	}

	const std::type_info *getOutputType() const override {
		return &typeid(output_t);
	}
};

template<typename T>
struct Proxy<ranges::optional<T>> : public OptionalProxy {
	using output_t = ranges::optional<T>;
	
	std::function<output_t(std::any &)> func;
	const std::type_info *sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const std::type_info *sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

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

	std::function<std::any(std::any &)> getGenericAny() const override {
		return [f = this->func](std::any &val) -> std::any {
			return f(val);
		};
	}

	const std::type_info *getSourceType() const override {
		return sourceType;
	}

	const std::type_info *getDestType() const override {
		return &typeid(output_t);
	}

	const std::type_info *getOutputType() const override {
		return &typeid(output_t);
	}
};

template<typename T>
struct Proxy<RawRange<T>> : public SequenceProxy<T>, public RangeProxy {
	using output_t = RawRange<T>;
	
	std::function<output_t(std::any &)> func;
	const std::type_info *sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const std::type_info *sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<RawRange<BlocksciType>(std::any &)> getGenericRange() const override {
		return [f = this->func](std::any &val) -> RawRange<BlocksciType> {
			return ranges::view::transform(f(val), [](T t) -> BlocksciType {
				return BlocksciType{t};
			});
		};
	}

	std::function<RawIterator<T>(std::any &)> getIteratorFunc() const override {
		return this->func;
	}

	const std::type_info *getSourceType() const override {
		return sourceType;
	}

	const std::type_info *getDestType() const override {
		return &typeid(output_t);
	}

	const std::type_info *getOutputType() const override {
		return &typeid(decltype(convertPythonRange(std::declval<output_t>())));
	}
};

template<typename T>
struct Proxy<RawIterator<T>> : public SequenceProxy<T>, public IteratorProxy {
	using output_t = RawIterator<T>;
	
	std::function<output_t(std::any &)> func;
	const std::type_info *sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const std::type_info *sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<RawIterator<BlocksciType>(std::any &)> getGenericIterator() const override {
		return [f = this->func](std::any &val) -> RawIterator<BlocksciType> {
			return ranges::view::transform(f(val), [](T t) -> BlocksciType {
				return BlocksciType{t};
			});
		};
	}

	std::function<RawIterator<T>(std::any &)> getIteratorFunc() const override {
		return this->func;
	}

	const std::type_info *getSourceType() const override {
		return sourceType;
	}

	const std::type_info *getDestType() const override {
		return &typeid(output_t);
	}

	const std::type_info *getOutputType() const override {
		return &typeid(decltype(convertPythonRange(std::declval<output_t>())));
	}
};

template<blocksci::AddressType::Enum type>
struct Proxy<blocksci::ScriptAddress<type>> : public ProxyAddress {
	using output_t = blocksci::ScriptAddress<type>;
	
	std::function<output_t(std::any &)> func;
	const std::type_info *sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const std::type_info *sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

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

	const std::type_info *getSourceType() const override {
		return sourceType;
	}

	const std::type_info *getDestType() const override {
		return &typeid(output_t);
	}

	const std::type_info *getOutputType() const override {
		return &typeid(output_t);
	}
};

template<>
struct Proxy<blocksci::AnyScript> : public ProxyAddress {
	using output_t = blocksci::AnyScript;
	
	std::function<output_t(std::any &)> func;
	const std::type_info *sourceType;

	Proxy(std::function<output_t(std::any &)> && func_, const std::type_info *sourceType_) : func(std::move(func_)), sourceType(sourceType_) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<blocksci::AnyScript(std::any &)> getGenericScript() const override {
		return this->func;
	}

	const std::type_info *getSourceType() const override {
		return sourceType;
	}

	const std::type_info *getDestType() const override {
		return &typeid(output_t);
	}

	const std::type_info *getOutputType() const override {
		return &typeid(output_t);
	}
};

#endif /* proxy_hpp */
