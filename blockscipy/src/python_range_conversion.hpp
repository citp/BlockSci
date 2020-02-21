//
//  python_range_conversion.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef python_range_conversion_h
#define python_range_conversion_h

#include "python_fwd.hpp"
#include "sequence.hpp"

#include <blocksci/blocksci_fwd.hpp>

#include <pybind11/numpy.h>

#include <chrono>

namespace pybind11 { namespace detail {
    template <>
    struct npy_format_descriptor<NumpyDatetime> { 
        static constexpr auto name = _("datetime64[ns]");
        static pybind11::dtype dtype() { return pybind11::dtype(std::string("datetime64[ns]")); }
    };

    template <>
    struct npy_format_descriptor<NumpyBool> { 
        static constexpr auto name = _("bool");
        static pybind11::dtype dtype() { return pybind11::dtype(std::string("bool")); }
    };
}}

struct PythonConversionTypeConverter {
    pybind11::list operator()(RawIterator<pybind11::bytes> && t);
    pybind11::list operator()(RawIterator<pybind11::list> && t);
    pybind11::list operator()(RawIterator<std::string> && t);
    pybind11::list operator()(RawIterator<blocksci::AddressType::Enum> && t);

    pybind11::list operator()(RawRange<pybind11::bytes> && t);
    pybind11::list operator()(RawRange<pybind11::list> && t);
    pybind11::list operator()(RawRange<std::string> && t);
    pybind11::list operator()(RawRange<blocksci::AddressType::Enum> && t);

    pybind11::array_t<int64_t> operator()(RawIterator<int64_t> && t);
    pybind11::array_t<uint64_t> operator()(RawIterator<uint64_t> && t);
    pybind11::array_t<int32_t> operator()(RawIterator<int32_t> && t);
    pybind11::array_t<uint32_t> operator()(RawIterator<uint32_t> && t);
    pybind11::array_t<int16_t> operator()(RawIterator<int16_t> && t);
    pybind11::array_t<uint16_t> operator()(RawIterator<uint16_t> && t);
    pybind11::array_t<NumpyBool> operator()(RawIterator<bool> && t);
    pybind11::array_t<NumpyDatetime> operator()(RawIterator<std::chrono::system_clock::time_point> && t);
    pybind11::array_t<std::array<char, 40>> operator()(RawIterator<blocksci::uint160> && t);
    pybind11::array_t<std::array<char, 64>> operator()(RawIterator<blocksci::uint256> && t);

    pybind11::array_t<int64_t> operator()(RawRange<int64_t> && t);
    pybind11::array_t<uint64_t> operator()(RawRange<uint64_t> && t);
    pybind11::array_t<int32_t> operator()(RawRange<int32_t> && t);
    pybind11::array_t<uint32_t> operator()(RawRange<uint32_t> && t);
    pybind11::array_t<int16_t> operator()(RawRange<int16_t> && t);
    pybind11::array_t<uint16_t> operator()(RawRange<uint16_t> && t);
    pybind11::array_t<NumpyBool> operator()(RawRange<bool> && t);
    pybind11::array_t<NumpyDatetime> operator()(RawRange<std::chrono::system_clock::time_point> && t);
    pybind11::array_t<std::array<char, 40>> operator()(RawRange<blocksci::uint160> && t);
    pybind11::array_t<std::array<char, 64>> operator()(RawRange<blocksci::uint256> && t);

    
    Iterator<blocksci::AnyScript> operator()(RawIterator<blocksci::Address> && t);
    Range<blocksci::AnyScript> operator()(RawRange<blocksci::Address> && t);

    template <typename T>
    Iterator<T> operator()(RawIterator<T> && t) {
        return t;
    }

    template <typename T>
    Range<T> operator()(RawRange<T> && t) {
        return t;
    }
};



template <typename T>
auto convertPythonRange(T && t) {
    return PythonConversionTypeConverter{}(std::move(t));
}

#endif /* range_conversion_h */
