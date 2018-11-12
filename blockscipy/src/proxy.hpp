//
//  proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_hpp
#define proxy_hpp

#include "python_fwd.hpp"
#include "blocksci_type.hpp"
#include "generic_sequence.hpp"

#include <range/v3/view/transform.hpp>

#include <any>
#include <functional>

enum class ProxyType {
	Simple, Optional, Iterator, Range
};

struct GenericProxy {
	virtual std::function<std::any(std::any &)> getGenericAny() const = 0;
	virtual ProxyType getProxyType() const = 0;
	virtual ~GenericProxy() = default;
};

struct SimpleProxy : public GenericProxy {
	virtual std::function<BlocksciType(std::any &)> getGenericSimple() const = 0;
	virtual ~SimpleProxy() = default;

	std::function<std::any(std::any &)> getGenericAny() const override {
		auto generic = getGenericSimple();
		return [generic](std::any &val) -> std::any {
			return generic(val).toAny();
		};
	}

	ProxyType getProxyType() const override {
		return ProxyType::Simple;
	}
};

struct IteratorProxy : public GenericProxy {
	virtual std::function<RawIterator<BlocksciType>(std::any &)> getGenericIterator() const = 0;
	virtual ~IteratorProxy() = default;

	virtual ProxyType getProxyType() const override {
		return ProxyType::Iterator;
	}

	std::function<std::any(std::any &)> getGenericAny() const override {
		auto generic = getGenericIterator();
		return [generic](std::any &val) -> std::any {
			return generic(val);
		};
	}

	std::function<RawIterator<BlocksciType>(std::any &)> getGeneric() const {
		return getGenericIterator();
	}
};

struct RangeProxy : public IteratorProxy {
	virtual std::function<RawRange<BlocksciType>(std::any &)> getGenericRange() const = 0;
	virtual ~RangeProxy() = default;

	ProxyType getProxyType() const override {
		return ProxyType::Range;
	}

	std::function<RawIterator<BlocksciType>(std::any &)> getGenericIterator() const override {
		auto generic = getGenericRange();
		return [generic](std::any &val) -> RawIterator<BlocksciType> {
			return generic(val);
		};
	}

	std::function<std::any(std::any &)> getGenericAny() const override {
		auto generic = getGenericRange();
		return [generic](std::any &val) -> std::any {
			return generic(val);
		};
	}

	std::function<RawRange<BlocksciType>(std::any &)> getGeneric() const {
		return getGenericRange();
	}
};

struct OptionalProxy : public GenericProxy {
	virtual std::function<ranges::optional<BlocksciType>(std::any &)> getGenericOptional() const = 0;
	virtual ~OptionalProxy() = default;

	ProxyType getProxyType() const override {
		return ProxyType::Optional;
	}

	std::function<ranges::optional<BlocksciType>(std::any &)> getGeneric() const {
		return getGenericOptional();
	}
};

template<typename T>
struct SequenceProxy {
	virtual std::function<RawIterator<T>(std::any &)> getIteratorFunc() const = 0;
	virtual ~SequenceProxy() = default;
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

	std::function<BlocksciType(std::any &)> getGenericSimple() const override {
		return [generic = getGenericScript()](std::any &val) -> BlocksciType {
			return BlocksciType{generic(val)};
		};
	}

	std::function<blocksci::AnyScript(std::any &)> getGeneric() const {
		return getGenericScript();
	}
};

template<typename T>
struct Proxy : public SimpleProxy {
	using output_t = T;
	
	std::function<output_t(std::any &)> func;

	Proxy(const std::function<output_t(std::any &)> &func_) : func(func_) {}

	Proxy(std::function<output_t(std::any &)> && func_) : func(std::move(func_)) {}

	Proxy(const output_t &val) : Proxy(std::function<output_t(std::any &)>{[val](std::any &) -> output_t {
		return val;
	}}) {}

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
};

template<typename T>
struct Proxy<ranges::optional<T>> : public OptionalProxy {
	using output_t = ranges::optional<T>;
	
	std::function<output_t(std::any &)> func;

	Proxy(const std::function<output_t(std::any &)> &func_) : func(func_) {}

	Proxy(std::function<output_t(std::any &)> && func_) : func(std::move(func_)) {}

	Proxy(const output_t &val) : Proxy(std::function<output_t(std::any &)>{[val](std::any &) -> output_t {
		return val;
	}}) {}

	Proxy(const Proxy<T> &p) : Proxy(std::function<ranges::optional<T>(std::any &)>{[p](std::any &v) -> ranges::optional<T> {
		return p(v);
	}}) {}

	Proxy(const T &val) : Proxy(std::function<ranges::optional<T>(std::any &)>{[val](std::any &) -> ranges::optional<T> {
		return val;
	}}) {}

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
};

template<typename T>
struct Proxy<RawRange<T>> : public SequenceProxy<T>, public RangeProxy {
	using output_t = RawRange<T>;
	
	std::function<output_t(std::any &)> func;

	Proxy(const std::function<output_t(std::any &)> &func_) : func(func_) {}

	Proxy(std::function<output_t(std::any &)> && func_) : func(std::move(func_)) {}

	Proxy(const output_t &val) : Proxy(std::function<output_t(std::any &)>{[val](std::any &) -> output_t {
		return val;
	}}) {}

	Proxy(const Range<T> &val) : Proxy(std::function<output_t(std::any &)>{[rng = val.rng](std::any &) -> output_t {
		return rng;
	}}) {}

	Range<T> operator()(std::any &t) const {
		return func(t);
	}

	Range<T> operator()(std::any && t) const {
		return func(t);
	}

	output_t applySimple(std::any &t) const {
		return func(t);
	}

	output_t applySimple(std::any && t) const {
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
};

template<typename T>
struct Proxy<RawIterator<T>> : public SequenceProxy<T>, public IteratorProxy {
	using output_t = RawIterator<T>;
	
	std::function<output_t(std::any &)> func;

	Proxy(const std::function<output_t(std::any &)> &func_) : func(func_) {}

	Proxy(std::function<output_t(std::any &)> && func_) : func(std::move(func_)) {}

	Proxy(const output_t &val) : Proxy(std::function<output_t(std::any &)>{[val](std::any &) -> output_t {
		return val;
	}}) {}

	Proxy(const Iterator<T> &val) : Proxy(std::function<output_t(std::any &)>{[rng = val.rng](std::any &) -> output_t {
		return rng;
	}}) {}

	Proxy(const Proxy<RawRange<T>> &p) : Proxy(std::function<output_t(std::any &)>{
		[p](std::any &v) -> output_t {
		return p.applySimple(v);
	}}) {}

	Proxy(Proxy<RawRange<T>> && p) : Proxy(std::function<output_t(std::any &)>{
		[p](std::any &v) -> output_t {
		return p.applySimple(v);
	}}) {}

	Iterator<T> operator()(std::any &t) const {
		return func(t);
	}

	Iterator<T> operator()(std::any && t) const {
		return func(t);
	}

	output_t applySimple(std::any &t) const {
		return func(t);
	}

	output_t applySimple(std::any && t) const {
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
};

template<blocksci::AddressType::Enum type>
struct Proxy<blocksci::ScriptAddress<type>> : public ProxyAddress {
	using output_t = blocksci::ScriptAddress<type>;
	
	std::function<output_t(std::any &)> func;

	Proxy(const std::function<output_t(std::any &)> &func_) : func(func_) {}

	Proxy(std::function<output_t(std::any &)> && func_) : func(std::move(func_)) {}

	Proxy(const output_t &val) : Proxy(std::function<output_t(std::any &)>{[val](std::any &) -> output_t {
		return val;
	}}) {}

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
};

template<>
struct Proxy<blocksci::AnyScript> : public ProxyAddress {
	using output_t = blocksci::AnyScript;
	
	std::function<output_t(std::any &)> func;

	Proxy(const std::function<output_t(std::any &)> &func_) : func(func_) {}

	Proxy(std::function<output_t(std::any &)> && func_) : func(std::move(func_)) {}

	Proxy(const output_t &val) : Proxy(std::function<output_t(std::any &)>{[val](std::any &) -> output_t {
		return val;
	}}) {}

	output_t operator()(std::any &t) const {
		return func(t);
	}

	output_t operator()(std::any && t) const {
		return func(t);
	}

	std::function<blocksci::AnyScript(std::any &)> getGenericScript() const override {
		return this->func;
	}
};

#endif /* proxy_hpp */
