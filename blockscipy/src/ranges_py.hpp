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


template <typename T, typename Class>
void addMapFunc(Class &cl) {
    using Range = typename Class::type;
    using value_type = ranges::range_value_type_t<Range>;
    cl
    .def("map", [](Range &range, Proxy<value_type, T> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(proxy));
    })
    .def("map", [](Range &range, Proxy<value_type, ranges::optional<T>> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(proxy));
    })
    ;
}

template <typename T, typename Class>
void addMapAnyRangeFunc(Class &cl) {
    using Range = typename Class::type;
    using value_type = ranges::range_value_type_t<Range>;

    addMapFunc<T>(cl);
    cl
    .def("map", [](Range &range, Proxy<value_type, ranges::any_view<T>> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(proxy));
    })
    .def("map", [](Range &range, Proxy<value_type, ranges::any_view<T, ranges::category::random_access | ranges::category::sized>> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(proxy));
    })
    // .def("map", [](Range &range, Proxy<value_type, ranges::any_view<ranges::optional<T>>> &proxy) {
    //     return convertRangeToPython(range | ranges::view::transform(proxy));
    // })
    // .def("map", [](Range &range, Proxy<value_type, ranges::any_view<ranges::optional<T>, ranges::category::random_access | ranges::category::sized>> &proxy) {
    //     return convertRangeToPython(range | ranges::view::transform(proxy));
    // })
    ;
}

template<typename Class>
auto addRangeMethods(Class &cl) {
    using Range = typename Class::type;
    using value_type = ranges::range_value_type_t<Range>;

    addGenericRangeMethods(cl);

    cl
    .def("where", [](Range &range, Proxy<value_type, bool> &proxy) {
        return convertRangeToPython(range | ranges::view::filter(proxy));
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
    return cl;
}



template <typename T>
void addAllRangeMethods(T &cls) {
    addRangeMethods(cls.iterator);
    addRangeMethods(cls.range);
}

#endif /* ranges_py_hpp */
