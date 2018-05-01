//
//  range_conversion.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "range_conversion.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/address/equiv_address.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>

#include <chrono>

using namespace blocksci;
namespace py = pybind11;

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

typename NumpyConverter<std::chrono::system_clock::time_point>::type NumpyConverter<std::chrono::system_clock::time_point>::operator()(const std::chrono::system_clock::time_point &val) {
    return {std::chrono::duration_cast<std::chrono::nanoseconds>(val.time_since_epoch()).count()};
}

typename NumpyConverter<blocksci::uint256>::type NumpyConverter<blocksci::uint256>::operator()(const blocksci::uint256 &val) {
    auto hexStr = val.GetHex();
    std::array<char, 64> ret;
    std::copy_n(hexStr.begin(), 64, ret.begin());
    return ret;
}

template <typename T>
pybind11::list convertRangeToPythonPy(T && t) {
    if constexpr (ranges::RandomAccessRange<T>()) {
        auto rangeSize = static_cast<size_t>(ranges::distance(t));
        pybind11::list list{rangeSize};
        RANGES_FOR(auto && a, t) {
            list.append(std::forward<decltype(a)>(a));
        }
        return list;
    } else {
        pybind11::list list;
        RANGES_FOR(auto && a, std::forward<T>(t)) {
            list.append(std::forward<decltype(a)>(a));
        }
        return list;
    }
}

template <typename T>
pybind11::array_t<typename NumpyConverter<ranges::range_value_type_t<T>>::type> convertRangeToPythonNumpy(T && t) {
    using numpy_converter = NumpyConverter<ranges::range_value_type_t<T>>;
    using numpy_value_type = typename numpy_converter::type;

    auto numpy_converted = std::forward<T>(t) | ranges::view::transform(numpy_converter{});
    if constexpr (ranges::RandomAccessRange<T>()) {
        auto rangeSize = static_cast<size_t>(ranges::distance(numpy_converted));
        pybind11::array_t<numpy_value_type> ret{rangeSize};
        auto retPtr = ret.mutable_data();
        ranges::copy(numpy_converted, retPtr);
        return ret;
    } else {
        auto ret = numpy_converted | ranges::to_vector;
        return pybind11::array_t<numpy_value_type>{ret.size(), ret.data()};
    }
}

template <typename T>
using blocksci_range_type_t = ranges::any_view<ranges::range_value_type_t<T>, getBlockSciCategory(ranges::get_categories<T>())>;

template <typename T>
blocksci_range_type_t<T> convertRangeToPythonBlockSci(T && t) {
    return {std::forward<T>(t)};
}

template <typename T>
converted_range_impl_t<T> convertRangeToPythonImpl(T && t) {
    auto converted = std::forward<T>(t) | ranges::view::transform([](auto && x) { return BasicTypeConverter{}(std::forward<decltype(x)>); });
    using range_tag = typename type_tag<ranges::range_value_type_t<decltype(converted)>>::type;

    if constexpr (std::is_same_v<range_tag, py_tag>) {
        return convertRangeToPythonPy(converted);
    } else if constexpr (std::is_same_v<range_tag, numpy_tag>) {
        return convertRangeToPythonNumpy(converted);
    } else if constexpr (std::is_same_v<range_tag, blocksci_tag>) {
        return convertRangeToPythonBlockSci(converted);
    }
}

template <typename T>
converted_range_t<T> convertAnyRangeToPython(T && t) {
	return convertRangeToPythonImpl(std::forward<T>(t));
}

template <typename T>
using random_range = ranges::any_view<T, ranges::category::random_access>;

#define createConvertedRangeTag(functype) \
	template converted_range_t<functype> convertAnyRangeToPython<functype>(functype &&);

#define createRangeFuncsImpl(functype) \
	createConvertedRangeTag(ranges::any_view<functype>) \
	createConvertedRangeTag(random_range<functype>) \

#define createRangeFuncs(functype) \
	createRangeFuncsImpl(functype) \
	createRangeFuncsImpl(ranges::optional<functype>) \

createRangeFuncs(AddressType::Enum)
createRangeFuncs(Input)
createRangeFuncs(Output)
createRangeFuncs(Transaction)
createRangeFuncs(Block)
createRangeFuncs(Address)
createRangeFuncs(AnyScript)
createRangeFuncs(EquivAddress)
createRangeFuncs(script::Pubkey)
createRangeFuncs(script::PubkeyHash)
createRangeFuncs(script::WitnessPubkeyHash)
createRangeFuncs(script::MultisigPubkey)
createRangeFuncs(script::Multisig)
createRangeFuncs(script::OpReturn)
createRangeFuncs(script::Nonstandard)
createRangeFuncs(script::ScriptHash)
createRangeFuncs(script::WitnessScriptHash)
createRangeFuncs(Cluster)
createRangeFuncs(TaggedCluster)
createRangeFuncs(TaggedAddress)
createRangeFuncs(uint160)
createRangeFuncs(uint256)
createRangeFuncs(uint16_t)
createRangeFuncs(int16_t)
createRangeFuncs(uint32_t)
createRangeFuncs(int32_t)
createRangeFuncs(uint64_t)
createRangeFuncs(int64_t)
createRangeFuncs(bool)
createRangeFuncs(std::string)
createRangeFuncs(std::chrono::system_clock::time_point)
createRangeFuncs(py::list)
createRangeFuncs(py::bytes)
using intAddressPair = std::pair<int64_t, Address>;
using addressIntPair = std::pair<Address, int64_t>;
createRangeFuncs(intAddressPair)
createRangeFuncs(addressIntPair)
