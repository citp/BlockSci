//
//  sequence_group.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/11/18.
//

#include "sequence_group.hpp"
#include "generic_sequence.hpp"
#include "proxy.hpp"
#include "caster_py.hpp"

#include <pybind11/stl.h>

template<typename GroupType, typename ResultType>
void addGroupByFunc(pybind11::class_<GenericIterator> &cl) {
    cl.def("_group_by", [](GenericIterator &range, Proxy<GroupType> &grouper, Proxy<ResultType> &eval) -> std::unordered_map<GroupType, ResultType> {
        std::unordered_map<GroupType, std::vector<BlocksciType>> grouped;
        RANGES_FOR(auto && item, range.getGenericIterator()) {
            grouped[grouper(item.toAny())].push_back(item);
        }
        std::unordered_map<GroupType, ResultType> results;
        results.reserve(grouped.size());
        for (auto &group : grouped) {
            auto range = RawRange<BlocksciType>{group.second};
            results[group.first] = eval(range);
        }
        return results;
    });
}

template<typename T>
void addGroupByFuncs(pybind11::class_<GenericIterator> &cl) {
    addGroupByFunc<int64_t, T>(cl);
    addGroupByFunc<blocksci::AddressType::Enum, T>(cl);
    addGroupByFunc<bool, T>(cl);
    addGroupByFunc<blocksci::AnyScript, T>(cl);
}

void addAllGroupMethods(pybind11::class_<GenericIterator> &cl) {
    addGroupByFuncs<int64_t>(cl);
    addGroupByFuncs<bool>(cl);
    addGroupByFuncs<std::chrono::system_clock::time_point>(cl);
    addGroupByFuncs<blocksci::AddressType::Enum>(cl);


    cl.def("to_list", [](GenericIterator & range) { 
        pybind11::list list;
        RANGES_FOR(auto && item, range.getGenericIterator()) {
            mpark::visit([&list](auto && a) {
                list.append(std::forward<decltype(a)>(a));
            }, item.var);
        }
        return list;
    }, "Returns a list of all of the objects in the range");
}
