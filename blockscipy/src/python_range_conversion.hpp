//
//  python_range_conversion.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef python_range_conversion_h
#define python_range_conversion_h

#include "generic_sequence.hpp"

#include <blocksci/blocksci_fwd.hpp>

#include <pybind11/numpy.h>

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
    pybind11::list operator()(Iterator<pybind11::bytes> && t);
    pybind11::list operator()(Iterator<pybind11::list> && t);
    pybind11::list operator()(Iterator<std::string> && t);
    pybind11::list operator()(Iterator<blocksci::AddressType::Enum> && t);

    pybind11::list operator()(Range<pybind11::bytes> && t);
    pybind11::list operator()(Range<pybind11::list> && t);
    pybind11::list operator()(Range<std::string> && t);
    pybind11::list operator()(Range<blocksci::AddressType::Enum> && t);

    pybind11::array_t<int64_t> operator()(Iterator<int64_t> && t);
    pybind11::array_t<uint64_t> operator()(Iterator<uint64_t> && t);
    pybind11::array_t<int32_t> operator()(Iterator<int32_t> && t);
    pybind11::array_t<uint32_t> operator()(Iterator<uint32_t> && t);
    pybind11::array_t<int16_t> operator()(Iterator<int16_t> && t);
    pybind11::array_t<uint16_t> operator()(Iterator<uint16_t> && t);
    pybind11::array_t<NumpyBool> operator()(Iterator<bool> && t);
    pybind11::array_t<NumpyDatetime> operator()(Iterator<std::chrono::system_clock::time_point> && t);
    pybind11::array_t<std::array<char, 40>> operator()(Iterator<blocksci::uint160> && t);
    pybind11::array_t<std::array<char, 64>> operator()(Iterator<blocksci::uint256> && t);

    pybind11::array_t<int64_t> operator()(Range<int64_t> && t);
    pybind11::array_t<uint64_t> operator()(Range<uint64_t> && t);
    pybind11::array_t<int32_t> operator()(Range<int32_t> && t);
    pybind11::array_t<uint32_t> operator()(Range<uint32_t> && t);
    pybind11::array_t<int16_t> operator()(Range<int16_t> && t);
    pybind11::array_t<uint16_t> operator()(Range<uint16_t> && t);
    pybind11::array_t<NumpyBool> operator()(Range<bool> && t);
    pybind11::array_t<NumpyDatetime> operator()(Range<std::chrono::system_clock::time_point> && t);
    pybind11::array_t<std::array<char, 40>> operator()(Range<blocksci::uint160> && t);
    pybind11::array_t<std::array<char, 64>> operator()(Range<blocksci::uint256> && t);

    Iterator<ranges::optional<int64_t>> operator()(Iterator<ranges::optional<int16_t>> && t);
    Iterator<ranges::optional<int64_t>> operator()(Iterator<ranges::optional<uint16_t>> && t);
    Iterator<ranges::optional<int64_t>> operator()(Iterator<ranges::optional<int32_t>> && t);
    Iterator<ranges::optional<int64_t>> operator()(Iterator<ranges::optional<uint32_t>> && t);
    Iterator<ranges::optional<int64_t>> operator()(Iterator<ranges::optional<uint64_t>> && t);
    Iterator<blocksci::AnyScript> operator()(Iterator<blocksci::Address> && t);
    Iterator<ranges::optional<blocksci::AnyScript>> operator()(Iterator<ranges::optional<blocksci::Address>> && t);

    Range<ranges::optional<int64_t>> operator()(Range<ranges::optional<int16_t>> && t);
    Range<ranges::optional<int64_t>> operator()(Range<ranges::optional<uint16_t>> && t);
    Range<ranges::optional<int64_t>> operator()(Range<ranges::optional<int32_t>> && t);
    Range<ranges::optional<int64_t>> operator()(Range<ranges::optional<uint32_t>> && t);
    Range<ranges::optional<int64_t>> operator()(Range<ranges::optional<uint64_t>> && t);
    Range<blocksci::AnyScript> operator()(Range<blocksci::Address> && t);
    Range<ranges::optional<blocksci::AnyScript>> operator()(Range<ranges::optional<blocksci::Address>> && t);

    template <typename T>
    Iterator<T> operator()(Iterator<T> && t) {
        return t;
    }

    template <typename T>
    Range<T> operator()(Range<T> && t) {
        return t;
    }
};



template <typename T>
auto convertPythonRange(T && t) {
    return PythonConversionTypeConverter{}(std::move(t));
}

#endif /* range_conversion_h */
