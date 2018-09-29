//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include "python_fwd.hpp"
#include "proxy.hpp"
#include "proxy_utils.hpp"
#include "ranges_common_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/functional.h>

#include <range/v3/size.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/view/transform.hpp>


template<typename T, typename V, ranges::category range_cat>
void addMapFunc(pybind11::class_<ranges::any_view<V, range_cat>> &cl) {
    using R = ranges::any_view<V, range_cat>;
    cl
    .def("_map", [](R &range, Proxy<V, T> &proxy) {
        return convertPythonRange(ranges::any_view<T, range_cat>{ranges::view::transform(range, proxy)});
    })
    .def("_map", [](R &range, Proxy<V, ranges::optional<T>> &proxy) {
        return convertRangeToPython(ranges::view::transform(range, proxy));
    })
    ;
}

template<typename T, typename V, ranges::category range_cat>
void addMapAnyRangeFunc(pybind11::class_<ranges::any_view<V, range_cat>> &cl) {
    using R = ranges::any_view<V, range_cat>;
    addMapFunc<T>(cl);
    cl
    .def("_map", [](R &range, Proxy<V, ranges::any_view<T>> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(proxy));
    })
    .def("_map", [](R &range, Proxy<V, ranges::any_view<T, ranges::category::random_access | ranges::category::sized>> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(proxy));
    })
    ;
}

template<typename GroupType, typename ResultType, typename V, ranges::category range_cat>
void addGroupByFunc(pybind11::class_<ranges::any_view<V, range_cat>> &cl) {
    using Range = ranges::any_view<V, range_cat>;
    cl.def("group_by", [](Range &range, Proxy<V, GroupType> &grouper, Proxy<Iterator<V>, ResultType> &eval) -> std::unordered_map<GroupType, ResultType> {
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

template<typename V, ranges::category range_cat>
auto addRangeMethods(pybind11::class_<ranges::any_view<V, range_cat>> &cl) {
    using Range = ranges::any_view<V, range_cat>;

    addGenericRangeMethods(cl);

    cl
    .def("where", [](Range &range, Proxy<V, bool> &proxy) -> Iterator<V> {
        return ranges::view::filter(range, proxy);
    })
    .def_property_readonly_static("nested_proxy", [](pybind11::object &) -> Proxy<V, V> {
        return makeProxy<V>();
    })
    ;

    addMapAnyRangeFunc<blocksci::Block>(cl);
    addMapAnyRangeFunc<blocksci::Transaction>(cl);
    addMapAnyRangeFunc<blocksci::Input>(cl);
    addMapAnyRangeFunc<blocksci::Output>(cl);
    addMapFunc<blocksci::AnyScript>(cl);
    addMapFunc<blocksci::AddressType::Enum>(cl);
    addMapFunc<int64_t>(cl);
    addMapFunc<bool>(cl);
    addMapFunc<std::chrono::system_clock::time_point>(cl);
    addMapFunc<blocksci::uint256>(cl);
    addMapFunc<blocksci::uint160>(cl);
    addMapFunc<pybind11::bytes>(cl);

    addGroupByFuncs<int64_t>(cl);
    addGroupByFuncs<bool>(cl);
    addGroupByFuncs<std::chrono::system_clock::time_point>(cl);
    addGroupByFuncs<blocksci::AddressType::Enum>(cl);

    return cl;
}



template <typename T>
void addAllRangeMethods(T &cls) {
    addRangeMethods(cls.iterator);
    addRangeMethods(cls.range);
}

#endif /* ranges_py_hpp */
