//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include "python_fwd.hpp"
#include "blocksci_range.hpp"
#include "proxy.hpp"
#include "generic_sequence.hpp"
#include "proxy_create.hpp"

#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>


template<typename GroupType, typename ResultType, typename V>
void addGroupByFunc(pybind11::class_<Sequence<V>> &cl) {
    cl.def("_group_by", [](Sequence<V> &range, Proxy<GroupType> &grouper, Proxy<ResultType> &eval) -> std::unordered_map<GroupType, ResultType> {
        std::unordered_map<GroupType, std::vector<V>> grouped;
        RANGES_FOR(auto && item, range.getIterator()) {
            grouped[grouper(item)].push_back(item);
        }
        std::unordered_map<GroupType, ResultType> results;
        results.reserve(grouped.size());
        for (auto &group : grouped) {
            auto range = RawRange<V>{group.second};
            results[group.first] = eval(range);
        }
        return results;
    });
}

template<typename T, typename V>
void addGroupByFuncs(pybind11::class_<Sequence<V>> &cl) {
    addGroupByFunc<int64_t, T>(cl);
    addGroupByFunc<blocksci::AddressType::Enum, T>(cl);
    addGroupByFunc<bool, T>(cl);
    addGroupByFunc<blocksci::AnyScript, T>(cl);
}

template <typename T, CONCEPT_REQUIRES_(!ranges::Range<T>())>
auto pythonAllType(T && t) {
    return std::forward<T>(t);
}

template <typename T, CONCEPT_REQUIRES_(ranges::Range<T>())>
auto pythonAllType(T && t) {
    pybind11::list list;
    RANGES_FOR(auto && a, std::forward<T>(t)) {
        list.append(pythonAllType(std::forward<decltype(a)>(a)));
    }
    return list;
}


template<typename Class>
auto addCommonIteratorMethods(Class &cl) {
    using R = typename Class::type;
    cl
    .def("__iter__", [](R &range) { 
        return pybind11::make_iterator(range.begin(), range.end()); 
    }, pybind11::keep_alive<0, 1>())
    .def("to_list", [](R & range) { 
        return pythonAllType(range.rng);
    }, "Returns a list of all of the objects in the range")
    ;
}

template<typename Class>
auto addCommonRangeMethods(Class &cl) {
    using R = typename Class::type;
    cl
    .def("__bool__", [](R &range) {
        return !ranges::empty(range.rng);
        
    })
    .def("__len__", [](R &range) {
        return range.rng.size();
    })
    ;
}

template <typename T>
void addAllRangeMethods(RangeClasses<T> &cls) {
    addCommonIteratorMethods(cls.iterator);
    addCommonIteratorMethods(cls.range);
    addCommonRangeMethods(cls.range);

    cls.iterator
    .def_property_readonly_static("self_proxy", [](pybind11::object &) -> Proxy<RawIterator<T>> {
        return makeIteratorProxy<T>();
    })
    ;

    cls.range
    .def_property_readonly_static("self_proxy", [](pybind11::object &) -> Proxy<RawRange<T>> {
        return makeRangeProxy<T>();
    })
    ;

    addGroupByFuncs<int64_t>(cls.sequence);
    addGroupByFuncs<bool>(cls.sequence);
    addGroupByFuncs<std::chrono::system_clock::time_point>(cls.sequence);
    addGroupByFuncs<blocksci::AddressType::Enum>(cls.sequence);
}

#endif /* ranges_py_hpp */
