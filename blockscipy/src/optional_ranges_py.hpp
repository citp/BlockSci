//
//  optional_ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef optional_ranges_py_hpp
#define optional_ranges_py_hpp

#include "proxy.hpp"
#include "ranges_common_py.hpp"
#include "optional_utils.hpp"

#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>

template<typename T, typename R, ranges::category range_cat>
void addOptionalMapFunc(pybind11::class_<ranges::any_view<ranges::optional<R>, range_cat>> &cl) {
    using Range = ranges::any_view<ranges::optional<R>, range_cat>;
    cl
    .def("map", [](Range &range, Proxy<R, T> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(MakeOptionalFunctor<Proxy<R, T>>{proxy}));
    })
    .def("map", [](Range &range, Proxy<R, ranges::optional<T>> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(MakeOptionalFunctor<Proxy<R, ranges::optional<T>>>{proxy}));
    })
    ;
}

template<typename T, typename R, ranges::category range_cat>
void addOptionalMapAnyRangeFunc(pybind11::class_<ranges::any_view<ranges::optional<R>, range_cat>> &cl) {
    using Range = ranges::any_view<ranges::optional<R>, range_cat>;
    addOptionalMapFunc<T>(cl);
    cl
    .def("map", [](Range &range, Proxy<R, ranges::any_view<T>> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(MakeOptionalFunctor<Proxy<R, ranges::any_view<T>>>{proxy}));
    })
    .def("map", [](Range &range, Proxy<R, ranges::any_view<T, ranges::category::random_access | ranges::category::sized>> &proxy) {
        return convertRangeToPython(range | ranges::view::transform(MakeOptionalFunctor<Proxy<R, ranges::any_view<T, ranges::category::random_access | ranges::category::sized>>>{proxy}));
    })
    ;
}

template<typename T, ranges::category range_cat>
auto addOptionalRangeMethods(pybind11::class_<ranges::any_view<ranges::optional<T>, range_cat>> &cl) {
    using Range = ranges::any_view<ranges::optional<T>, range_cat>;

    addGenericRangeMethods(cl);

    std::stringstream ss;
    ss << "\n\n:type: :class:`" << PythonTypeName<ranges::any_view<T, range_cat>>::name() << "`";
    
    std::stringstream ss2;
    ss2 << "\n\n:type: :class:`numpy.ndarray[bool]`";    
    cl
    .def_property_readonly("has_value", [](Range &range) {
        return convertRangeToPython(range | ranges::view::transform([](auto && val) { return val.has_value(); }));
    }, strdup((std::string("Return a array of bools denoting whether a item in the sequence has a value or is none") + ss2.str()).c_str()))
    .def_property_readonly("with_value", [](Range &range) {
        return convertRangeToPython(range | ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
        | ranges::view::transform([](const auto &optional) { return *optional; }));
    }, strdup((std::string("Returns a sequence containing only non-None items in the sequence") + ss.str()).c_str()))
    .def("with_default_value", [](Range &range, const T &defVal) {
        return convertRangeToPython(range | ranges::view::transform([=](const auto &optional) {
            if (optional) {
                return *optional;
            } else {
                return defVal;
            }
        }));
    }, pybind11::arg("default_value"), strdup((std::string("Replace all none values in the sequence with the provided default value and return the resulting sequence") + ss.str()).c_str()))
    ;

    addOptionalMapAnyRangeFunc<blocksci::Block>(cl);
    addOptionalMapAnyRangeFunc<blocksci::Transaction>(cl);
    addOptionalMapAnyRangeFunc<blocksci::Input>(cl);
    addOptionalMapAnyRangeFunc<blocksci::Output>(cl);
    addOptionalMapFunc<blocksci::AnyScript>(cl);
    addOptionalMapFunc<blocksci::AddressType::Enum>(cl);
    addOptionalMapFunc<int64_t>(cl);
    addOptionalMapFunc<bool>(cl);
    addOptionalMapFunc<std::chrono::system_clock::time_point>(cl);
    addOptionalMapFunc<blocksci::uint256>(cl);
    addOptionalMapFunc<blocksci::uint160>(cl);
    addOptionalMapFunc<pybind11::bytes>(cl);
    return cl;
}

template <typename T>
void addAllOptionalRangeMethods(T &cls) {
    addOptionalRangeMethods(cls.optionalIterator);
    addOptionalRangeMethods(cls.optionalRange);
}

#endif /* optional_ranges_py_hpp */
