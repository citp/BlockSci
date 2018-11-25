//
//  generic_proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef generic_proxy_hpp
#define generic_proxy_hpp

#include "python_fwd.hpp"
#include "blocksci_type.hpp"
#include "sequence.hpp"
#include "proxy_type_check.hpp"

#include <range/v3/view/empty.hpp>
#include <range/v3/view/single.hpp>

#include <any>
#include <functional>

struct GenericProxy {
	virtual std::function<std::any(std::any &)> getGenericAny() const = 0;
	virtual ProxyType getProxyType() const = 0;
	virtual ProxyTypeInfo getSourceType() const = 0;
	virtual ProxyTypeInfo getDestType() const = 0;
	virtual const std::type_info *getOutputType() const = 0;
	virtual ~GenericProxy() = default;
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

struct OptionalProxy : public RangeProxy {
	virtual std::function<ranges::optional<BlocksciType>(std::any &)> getGenericOptional() const = 0;
	virtual ~OptionalProxy() = default;

	ProxyType getProxyType() const override {
		return ProxyType::Optional;
	}

	std::function<ranges::optional<BlocksciType>(std::any &)> getGeneric() const {
		return getGenericOptional();
	}

	std::function<RawRange<BlocksciType>(std::any &)> getGenericRange() const override {
		return [generic = getGenericOptional()](std::any &v) -> RawRange<BlocksciType> {
			auto val = generic(v);
            if (val) {
                return ranges::view::single(*val);
            } else {
                return ranges::view::empty<BlocksciType>();
            }
		};
	}
};

struct SimpleProxy : public OptionalProxy {
	virtual std::function<BlocksciType(std::any &)> getGenericSimple() const = 0;
	virtual ~SimpleProxy() = default;

	std::function<std::any(std::any &)> getGenericAny() const override {
		auto generic = getGenericSimple();
		return [generic](std::any &val) -> std::any {
			return generic(val).toAny();
		};
	}

	std::function<ranges::optional<BlocksciType>(std::any &)> getGenericOptional() const override {
		return [generic = getGenericSimple()](std::any &val) -> ranges::optional<BlocksciType> {
			return generic(val);
		};
	}

	ProxyType getProxyType() const override {
		return ProxyType::Simple;
	}
};

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

#endif /* generic_proxy_hpp */
