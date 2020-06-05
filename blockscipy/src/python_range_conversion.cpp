//
//  python_range_conversion.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_range_conversion.hpp"
#include "blocksci_type_converter.hpp"
#include "sequence.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/address/equiv_address.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>

#include <range/v3/range_for.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/copy.hpp>

#include <chrono>

using namespace blocksci;
namespace py = pybind11;

struct NumpyConverter {

    template <typename T>
    T operator()(const T &val) {
        return val;
    }

    NumpyDatetime operator()(const std::chrono::system_clock::time_point &val) {
        return {std::chrono::duration_cast<std::chrono::nanoseconds>(val.time_since_epoch()).count()};
    }

    std::array<char, 64> operator()(const blocksci::uint256 &val) {
        auto hexStr = val.GetHex();
        std::array<char, 64> ret;
        std::copy_n(hexStr.begin(), 64, ret.begin());
        return ret;
    }

    std::array<char, 40> operator()(const blocksci::uint160 &val) {
        auto hexStr = val.GetHex();
        std::array<char, 40> ret;
        std::copy_n(hexStr.begin(), 40, ret.begin());
        return ret;
    }

    NumpyBool operator()(const bool &val) {
        return {val};
    }
};

template <typename T>
pybind11::array_t<decltype(NumpyConverter{}(std::declval<ranges::range_value_type_t<T>>()))>
convertRandomSizedNumpy(T && t) {
    auto numpy_converted = ranges::views::transform(std::move(t), NumpyConverter{});
    auto rangeSize = static_cast<size_t>(ranges::size(numpy_converted));
    pybind11::array_t<ranges::range_value_type_t<decltype(numpy_converted)>> ret{rangeSize};
    auto retPtr = ret.mutable_data();
    ranges::copy(numpy_converted, retPtr);
    return ret;
}

template <typename T>
pybind11::array_t<decltype(NumpyConverter{}(std::declval<ranges::range_value_type_t<T>>()))>
convertInputNumpy(T && t) {
    auto ret = ranges::to_vector(ranges::views::transform(std::move(t), NumpyConverter{}));
    return pybind11::array_t<typename decltype(ret)::value_type>{ret.size(), ret.data()};
}

template <typename T>
py::list convertRandomSizedPy(T && t) {
    auto rangeSize = static_cast<size_t>(ranges::size(t));
    pybind11::list list{rangeSize};
    size_t index = 0;
    RANGES_FOR(auto && a, t) {
        list[index] = std::forward<decltype(a)>(a);
        index++;
    }
    return list;
}

template <typename T>
py::list convertInputPy(T && t) {
    pybind11::list list;
    RANGES_FOR(auto && a, std::move(t)) {
        list.append(std::forward<decltype(a)>(a));
    }
    return list;
}

template <typename T>
Iterator<decltype(BlockSciTypeConverter{}(std::declval<ranges::range_value_type_t<T>>()))>
convertInputBlockSci(T && t) {
    return ranges::any_view<decltype(BlockSciTypeConverter{}(std::declval<ranges::range_value_type_t<T>>()))>{ranges::views::transform(std::forward<T>(t), BlockSciTypeConverter{})};
}

template <typename T>
Range<decltype(BlockSciTypeConverter{}(std::declval<ranges::range_value_type_t<T>>()))>
convertRandomSizedBlockSci(T && t) {
    return ranges::any_view<decltype(BlockSciTypeConverter{}(std::declval<ranges::range_value_type_t<T>>())), random_access_sized>{ranges::views::transform(std::forward<T>(t), BlockSciTypeConverter{})};
}


Iterator<blocksci::AnyScript> PythonConversionTypeConverter::operator()(RawIterator<blocksci::Address> && t) { return convertInputBlockSci(std::move(t)); }
Range<blocksci::AnyScript> PythonConversionTypeConverter::operator()(RawRange<blocksci::Address> && t) { return convertRandomSizedBlockSci(std::move(t)); }

pybind11::list PythonConversionTypeConverter::operator()(RawIterator<pybind11::bytes> && t) { return convertInputPy(std::move(t)); }
pybind11::list PythonConversionTypeConverter::operator()(RawIterator<pybind11::list> && t) { return convertInputPy(std::move(t)); }
pybind11::list PythonConversionTypeConverter::operator()(RawIterator<std::string> && t) { return convertInputPy(std::move(t)); }
pybind11::list PythonConversionTypeConverter::operator()(RawIterator<blocksci::AddressType::Enum> && t) { return convertInputPy(std::move(t)); }

pybind11::list PythonConversionTypeConverter::operator()(RawRange<pybind11::bytes> && t) { return convertRandomSizedPy(std::move(t)); }
pybind11::list PythonConversionTypeConverter::operator()(RawRange<pybind11::list> && t) { return convertRandomSizedPy(std::move(t)); }
pybind11::list PythonConversionTypeConverter::operator()(RawRange<std::string> && t) { return convertRandomSizedPy(std::move(t)); }
pybind11::list PythonConversionTypeConverter::operator()(RawRange<blocksci::AddressType::Enum> && t) { return convertRandomSizedPy(std::move(t)); }

pybind11::array_t<int64_t> PythonConversionTypeConverter::operator()(RawIterator<int64_t> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<uint64_t> PythonConversionTypeConverter::operator()(RawIterator<uint64_t> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<int32_t> PythonConversionTypeConverter::operator()(RawIterator<int32_t> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<uint32_t> PythonConversionTypeConverter::operator()(RawIterator<uint32_t> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<int16_t> PythonConversionTypeConverter::operator()(RawIterator<int16_t> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<uint16_t> PythonConversionTypeConverter::operator()(RawIterator<uint16_t> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<NumpyBool> PythonConversionTypeConverter::operator()(RawIterator<bool> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<NumpyDatetime> PythonConversionTypeConverter::operator()(RawIterator<std::chrono::system_clock::time_point> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<std::array<char, 40>> PythonConversionTypeConverter::operator()(RawIterator<uint160> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<std::array<char, 64>> PythonConversionTypeConverter::operator()(RawIterator<uint256> && t) { return convertInputNumpy(std::move(t)); }

pybind11::array_t<int64_t> PythonConversionTypeConverter::operator()(RawRange<int64_t> && t) { return convertRandomSizedNumpy(std::move(t)); }
pybind11::array_t<uint64_t> PythonConversionTypeConverter::operator()(RawRange<uint64_t> && t) { return convertRandomSizedNumpy(std::move(t)); }
pybind11::array_t<int32_t> PythonConversionTypeConverter::operator()(RawRange<int32_t> && t) { return convertRandomSizedNumpy(std::move(t)); }
pybind11::array_t<uint32_t> PythonConversionTypeConverter::operator()(RawRange<uint32_t> && t) { return convertRandomSizedNumpy(std::move(t)); }
pybind11::array_t<int16_t> PythonConversionTypeConverter::operator()(RawRange<int16_t> && t) { return convertRandomSizedNumpy(std::move(t)); }
pybind11::array_t<uint16_t> PythonConversionTypeConverter::operator()(RawRange<uint16_t> && t) { return convertRandomSizedNumpy(std::move(t)); }
pybind11::array_t<NumpyBool> PythonConversionTypeConverter::operator()(RawRange<bool> && t) { return convertRandomSizedNumpy(std::move(t)); }
pybind11::array_t<NumpyDatetime> PythonConversionTypeConverter::operator()(RawRange<std::chrono::system_clock::time_point> && t) { return convertInputNumpy(std::move(t)); }
pybind11::array_t<std::array<char, 40>> PythonConversionTypeConverter::operator()(RawRange<uint160> && t) { return convertRandomSizedNumpy(std::move(t)); }
pybind11::array_t<std::array<char, 64>> PythonConversionTypeConverter::operator()(RawRange<uint256> && t) { return convertRandomSizedNumpy(std::move(t)); }
