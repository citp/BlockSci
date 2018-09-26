//
//  python_range_conversion.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef python_range_conversion_h
#define python_range_conversion_h

#include "range_utils.hpp"

#include <blocksci/blocksci_fwd.hpp>

#include <pybind11/numpy.h>

#include <range/v3/view/any_view.hpp>

#include <chrono>

struct NumpyBool {
    bool val;
};

struct NumpyDatetime {
    int64_t time;
};

namespace pybind11 { namespace detail {
    template <>
    struct npy_format_descriptor<NumpyDatetime> { 
        static PYBIND11_DESCR name() { return _("datetime64[ns]"); }
        static pybind11::dtype dtype() { return pybind11::dtype(std::string("datetime64[ns]")); }
    };

    template <>
    struct npy_format_descriptor<NumpyBool> { 
        static PYBIND11_DESCR name() { return _("bool"); }
        static pybind11::dtype dtype() { return pybind11::dtype(std::string("bool")); }
    };
}}

struct PythonConversionTypeConverter {
    pybind11::list operator()(ranges::any_view<pybind11::bytes> && t);
    pybind11::list operator()(ranges::any_view<pybind11::list> && t);
    pybind11::list operator()(ranges::any_view<std::string> && t);
    pybind11::list operator()(ranges::any_view<blocksci::AddressType::Enum> && t);

    pybind11::list operator()(ranges::any_view<pybind11::bytes, random_access_sized> && t);
    pybind11::list operator()(ranges::any_view<pybind11::list, random_access_sized> && t);
    pybind11::list operator()(ranges::any_view<std::string, random_access_sized> && t);
    pybind11::list operator()(ranges::any_view<blocksci::AddressType::Enum, random_access_sized> && t);

    pybind11::array_t<int64_t> operator()(ranges::any_view<int64_t> && t);
    pybind11::array_t<uint64_t> operator()(ranges::any_view<uint64_t> && t);
    pybind11::array_t<int32_t> operator()(ranges::any_view<int32_t> && t);
    pybind11::array_t<uint32_t> operator()(ranges::any_view<uint32_t> && t);
    pybind11::array_t<int16_t> operator()(ranges::any_view<int16_t> && t);
    pybind11::array_t<uint16_t> operator()(ranges::any_view<uint16_t> && t);
    pybind11::array_t<NumpyBool> operator()(ranges::any_view<bool> && t);
    pybind11::array_t<NumpyDatetime> operator()(ranges::any_view<std::chrono::system_clock::time_point> && t);
    pybind11::array_t<std::array<char, 40>> operator()(ranges::any_view<blocksci::uint160> && t);
    pybind11::array_t<std::array<char, 64>> operator()(ranges::any_view<blocksci::uint256> && t);

    pybind11::array_t<int64_t> operator()(ranges::any_view<int64_t, random_access_sized> && t);
    pybind11::array_t<uint64_t> operator()(ranges::any_view<uint64_t, random_access_sized> && t);
    pybind11::array_t<int32_t> operator()(ranges::any_view<int32_t, random_access_sized> && t);
    pybind11::array_t<uint32_t> operator()(ranges::any_view<uint32_t, random_access_sized> && t);
    pybind11::array_t<int16_t> operator()(ranges::any_view<int16_t, random_access_sized> && t);
    pybind11::array_t<uint16_t> operator()(ranges::any_view<uint16_t, random_access_sized> && t);
    pybind11::array_t<NumpyBool> operator()(ranges::any_view<bool, random_access_sized> && t);
    pybind11::array_t<NumpyDatetime> operator()(ranges::any_view<std::chrono::system_clock::time_point, random_access_sized> && t);
    pybind11::array_t<std::array<char, 40>> operator()(ranges::any_view<blocksci::uint160, random_access_sized> && t);
    pybind11::array_t<std::array<char, 64>> operator()(ranges::any_view<blocksci::uint256, random_access_sized> && t);

    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<int16_t>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<uint16_t>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<int32_t>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<uint32_t>> && t);
    ranges::any_view<ranges::optional<int64_t>> operator()(ranges::any_view<ranges::optional<uint64_t>> && t);
    ranges::any_view<blocksci::AnyScript> operator()(ranges::any_view<blocksci::Address> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>> operator()(ranges::any_view<ranges::optional<blocksci::Address>> && t);

    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<int16_t>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<uint16_t>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<int32_t>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<uint32_t>, random_access_sized> && t);
    ranges::any_view<ranges::optional<int64_t>, random_access_sized> operator()(ranges::any_view<ranges::optional<uint64_t>, random_access_sized> && t);
    ranges::any_view<blocksci::AnyScript, random_access_sized> operator()(ranges::any_view<blocksci::Address, random_access_sized> && t);
    ranges::any_view<ranges::optional<blocksci::AnyScript>, random_access_sized> operator()(ranges::any_view<ranges::optional<blocksci::Address>, random_access_sized> && t);

    template <typename T>
    ranges::any_view<T> operator()(ranges::any_view<T> && t) {
        return t;
    }

    template <typename T>
    ranges::any_view<T, random_access_sized> operator()(ranges::any_view<T, random_access_sized> && t) {
        return t;
    }
};



template <typename T>
auto convertPythonRange(T && t) {
    return PythonConversionTypeConverter{}(std::move(t));
}

#endif /* range_conversion_h */
