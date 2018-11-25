//
//  blocksci_type.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/24/18.
//

#include "blocksci_type.hpp"

struct BlocksciTypeHasher {
    size_t operator()(const std::chrono::system_clock::time_point &time) const {
        return std::chrono::time_point_cast<std::chrono::milliseconds>(time).time_since_epoch().count();
    }

    size_t operator()(const pybind11::list &list) const {
        return reinterpret_cast<size_t>(list.ptr());
    }

    size_t operator()(const pybind11::bytes &bytes) const {
        return std::hash<std::string>{}(bytes);
    }

    template <typename T>
    size_t operator()(const T &t) const {
        return std::hash<T>{}(t);
    }
};

struct BlocksciTypeObjectCaster {
    pybind11::object operator()(const pybind11::bytes &o) {
        return o;
    }

    pybind11::object operator()(const pybind11::list &o) {
        return o;
    }

    pybind11::object operator()(const blocksci::AnyScript &o) {
        return mpark::visit([](const auto &r) -> pybind11::object {
            return pybind11::cast(r);
        }, o.wrapped);
    }

    template <typename T>
    pybind11::object operator()(const T &o) {
        return pybind11::cast(o);
    }
};

struct BlocksciTypeEqual {
    bool operator()(const pybind11::bytes &a, const pybind11::bytes &b) const {
        return a.is(b);
    }

    bool operator()(const pybind11::list &a, const pybind11::list &b) const {
        return a.is(b);
    }

    template <typename T>
    bool operator()(const T &a, const T &b) const {
        return a == b;
    }

    template <typename T, typename U>
    bool operator()(const T &, const U &) const {
        return false;
    }
};

std::any BlocksciType::toAny() const {
    return mpark::visit([&](auto &r) -> std::any { return r; }, var);
}

pybind11::object BlocksciType::toObject() const {
    return mpark::visit(BlocksciTypeObjectCaster{}, var);
}

bool BlocksciType::operator==(const BlocksciType &o) const {
    return mpark::visit(BlocksciTypeEqual{}, var, o.var);
}

void addToList(pybind11::list &l, BlocksciType &it) {
    return mpark::visit([&](auto &r) { l.append(r); }, it.var);
}

namespace std {
    size_t hash<BlocksciType>::operator()(const BlocksciType &o) const {
        return mpark::visit(BlocksciTypeHasher{}, o.var);
    }
}