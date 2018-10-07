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
#include "range_utils.hpp"

#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>


template<typename GroupType, typename ResultType, typename V, ranges::category range_cat>
void addGroupByFunc(pybind11::class_<ranges::any_view<V, range_cat>> &cl) {
    using Range = ranges::any_view<V, range_cat>;
    cl.def("group_by", [](Range &range, Proxy<GroupType> &grouper, Proxy<ResultType> &eval) -> std::unordered_map<GroupType, ResultType> {
        std::unordered_map<GroupType, std::vector<V>> grouped;
        RANGES_FOR(auto && item, range) {
            grouped[grouper(item)].push_back(item);
        }
        std::unordered_map<GroupType, ResultType> results;
        for (auto group : grouped) {
            results[group.first] = eval(group.second);
        }
        return results;
    });
}

template<typename T, typename V, ranges::category range_cat>
void addGroupByFuncs(pybind11::class_<ranges::any_view<V, range_cat>> &cl) {
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
    .def_property_readonly("all", [](R & range) { 
        return pythonAllType(range);
    }, "Returns a list of all of the objects in the range")
    ;
}

template<typename Class>
auto addCommonRangeMethods(Class &cl) {
    using R = typename Class::type;
    cl
    .def("__bool__", [](R &range) {
        return !ranges::empty(range);
        
    })
    .def("__len__", [](R &range) {
        return range.size();
    })
    ;
}

template<typename Class>
auto addSequenceMethods(Class &cl) {
    using Range = typename Class::type;
    using V = ranges::range_value_type_t<Range>;

    cl
    .def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<V> {
        return makeProxy<V>();
    })
    .def_property_readonly_static("self_proxy", [](pybind11::object &) -> Proxy<Range> {
        return makeProxy<Range>();
    })
    ;

    addGroupByFuncs<int64_t>(cl);
    addGroupByFuncs<bool>(cl);
    addGroupByFuncs<std::chrono::system_clock::time_point>(cl);
    addGroupByFuncs<blocksci::AddressType::Enum>(cl);

    return cl;
}

template <typename T>
void addAllRangeMethods(T &cls) {
    addCommonIteratorMethods(cls.iterator);
    addCommonIteratorMethods(cls.range);
    addCommonRangeMethods(cls.range);
    addSequenceMethods(cls.iterator);
    addSequenceMethods(cls.range);
}

#endif /* ranges_py_hpp */
