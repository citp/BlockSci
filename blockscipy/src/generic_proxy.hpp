//
//  generic_proxy.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef generic_proxy_hpp
#define generic_proxy_hpp

#include "python_fwd.hpp"

#include <blocksci/scripts/scripts_fwd.hpp>
#include <range/v3/utility/optional.hpp>

#include <any>
#include <functional>

struct GenericProxy {
	virtual std::function<std::any(std::any &)> getGenericAny() const = 0;
	virtual ProxyType getProxyType() const = 0;
	virtual ProxyTypeInfo getSourceType() const = 0;
	virtual ProxyTypeInfo getDestType() const = 0;
	virtual ~GenericProxy() = default;
};

struct IteratorProxy : public GenericProxy {
	virtual std::function<BlocksciIteratorType(std::any &)> getGenericIterator() const = 0;
	virtual ~IteratorProxy() = default;

	virtual ProxyType getProxyType() const override {
		return ProxyType::Iterator;
	}

	std::function<std::any(std::any &)> getGenericAny() const override;

	std::function<BlocksciIteratorType(std::any &)> getGeneric() const {
		return getGenericIterator();
	}
};

struct RangeProxy : public IteratorProxy {
	virtual std::function<BlocksciRangeType(std::any &)> getGenericRange() const = 0;
	virtual ~RangeProxy() = default;

	ProxyType getProxyType() const override {
		return ProxyType::Range;
	}

	std::function<BlocksciIteratorType(std::any &)> getGenericIterator() const override;
	std::function<std::any(std::any &)> getGenericAny() const override;

	std::function<BlocksciRangeType(std::any &)> getGeneric() const {
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
};

struct SimpleProxy : public OptionalProxy {
	virtual std::function<BlocksciType(std::any &)> getGenericSimple() const = 0;
	virtual ~SimpleProxy() = default;

	std::function<std::any(std::any &)> getGenericAny() const override;
	std::function<ranges::optional<BlocksciType>(std::any &)> getGenericOptional() const override;
	std::function<BlocksciRangeType(std::any &)> getGenericRange() const override;

	ProxyType getProxyType() const override {
		return ProxyType::Simple;
	}
};

struct ProxyAddress : public SimpleProxy {
	virtual std::function<blocksci::AnyScript(std::any &)> getGenericScript() const = 0;
	virtual ~ProxyAddress() = default;

	std::function<BlocksciType(std::any &)> getGenericSimple() const override;

	std::function<blocksci::AnyScript(std::any &)> getGeneric() const {
		return getGenericScript();
	}
};

#endif /* generic_proxy_hpp */
