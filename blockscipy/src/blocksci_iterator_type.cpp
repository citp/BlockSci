//
//  blocksci_iterator_type.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/25/18.
//

#include "blocksci_type.hpp"
#include "blocksci_iterator_type.hpp"
#include "caster_py.hpp"

struct BlocksciIteratorTypeObjectCaster {
    template <typename T>
    pybind11::object operator()(const T &o) {
        return pybind11::cast(o);
    }
};

std::any BlocksciIteratorType::toAny() const {
    return mpark::visit([&](auto &r) -> std::any { return r; }, var);
}

pybind11::object BlocksciIteratorType::toObject() const {
    return mpark::visit(BlocksciIteratorTypeObjectCaster{}, var);
}

RawIterator<BlocksciType> BlocksciIteratorType::toGeneric() {
    return mpark::visit([&](auto &r) -> RawIterator<BlocksciType> { 
        return r | ranges::views::transform([](auto && v) -> BlocksciType {
            return BlocksciType{std::forward<decltype(v)>(v)};
        });
    }, var);
}

RawIterator<std::any> BlocksciIteratorType::toAnySequence() {
    return mpark::visit([&](auto &r) -> RawIterator<std::any> { 
        return r | ranges::views::transform([](auto && v) -> std::any {
            return std::forward<decltype(v)>(v);
        });
    }, var);
}
