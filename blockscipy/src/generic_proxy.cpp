//
//  generic_proxy.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/25/18.
//

#include "generic_proxy.hpp"
#include "caster_py.hpp"

#include "blocksci_type.hpp"
#include "blocksci_iterator_type.hpp"
#include "blocksci_range_type.hpp"

#include <range/v3/view/single.hpp>

std::function<std::any(std::any &)> IteratorProxy::getGenericAny() const {
	auto generic = getGenericIterator();
	return [generic](std::any &val) -> std::any {
		return generic(val).toAny();
	};
}

std::function<BlocksciIteratorType(std::any &)> RangeProxy::getGenericIterator() const {
	return [generic = getGenericRange()](std::any &val) -> BlocksciIteratorType {
		return generic(val).toIterator();
	};
}

std::function<std::any(std::any &)> RangeProxy::getGenericAny() const {
	auto generic = getGenericRange();
	return [generic](std::any &val) -> std::any {
		return generic(val).toAny();
	};
}

std::function<std::any(std::any &)> SimpleProxy::getGenericAny() const {
	auto generic = getGenericSimple();
	return [generic](std::any &val) -> std::any {
		return generic(val).toAny();
	};
}

std::function<ranges::optional<BlocksciType>(std::any &)> SimpleProxy::getGenericOptional() const {
	return [generic = getGenericSimple()](std::any &val) -> ranges::optional<BlocksciType> {
		return generic(val);
	};
}

std::function<BlocksciRangeType(std::any &)> SimpleProxy::getGenericRange() const {
	return [generic = getGenericSimple()](std::any &v) -> BlocksciRangeType {
		return mpark::visit([](auto && val) -> BlocksciRangeType {
			return BlocksciRangeType{RawRange<std::decay_t<decltype(val)>>{ranges::views::single(std::forward<decltype(val)>(val))}};
		}, generic(v).var);
	};
}

std::function<BlocksciType(std::any &)> ProxyAddress::getGenericSimple() const {
		return [generic = getGenericScript()](std::any &val) -> BlocksciType {
			return BlocksciType{generic(val)};
		};
	}
