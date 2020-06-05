//
//  blocksci_range_type.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/25/18.
//

#include "blocksci_type.hpp"
#include "blocksci_range_type.hpp"
#include "blocksci_iterator_type.hpp"
#include "caster_py.hpp"

struct BlocksciRangeTypeObjectCaster {
    template <typename T>
    pybind11::object operator()(const T &o) {
        return pybind11::cast(o);
    }
};

struct BlockSciRangeToIterator {
	template <typename T>
    BlocksciIteratorType operator()(T && t) {
    	return BlocksciIteratorType{RawIterator<ranges::range_value_type_t<T>>{std::forward<T>(t)}};
    }
};

std::any BlocksciRangeType::toAny() const {
    return mpark::visit([&](auto &r) -> std::any { return r; }, var);
}

pybind11::object BlocksciRangeType::toObject() const {
    return mpark::visit(BlocksciRangeTypeObjectCaster{}, var);
}

BlocksciIteratorType BlocksciRangeType::toIterator() {
	return mpark::visit(BlockSciRangeToIterator{}, var);
}

RawRange<BlocksciType> BlocksciRangeType::toGeneric() {
	return mpark::visit([&](auto &r) -> RawRange<BlocksciType> { 
		return r | ranges::views::transform([](auto && v) -> BlocksciType {
			return BlocksciType{std::forward<decltype(v)>(v)};
		});
	}, var);
}

RawRange<std::any> BlocksciRangeType::toAnySequence() {
    return mpark::visit([&](auto &r) -> RawRange<std::any> { 
        return r | ranges::views::transform([](auto && v) -> std::any {
            return std::forward<decltype(v)>(v);
        });
    }, var);
}
