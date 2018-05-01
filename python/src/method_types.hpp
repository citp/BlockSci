//
//  method_types.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/27/18.
//

#ifndef method_types_h
#define method_types_h

#include <blocksci/blocksci_fwd.hpp>

#include <range/v3/view/any_view.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <chrono>
#include <cstddef>

struct NumpyBool;
struct NumpyDatetime;

std::string getTypeName(const char *text, const std::type_info *const *types);

template <typename T>
struct PythonTypeName;

template <>
struct PythonTypeName<blocksci::AnyScript> {
	static std::string name() {
		return "AnyAddress";
	}
};

template <>
struct PythonTypeName<blocksci::script::Pubkey> {
	static std::string name() {
		return "PubkeyAddress";
	}
};

template <>
struct PythonTypeName<blocksci::script::PubkeyHash> {
	static std::string name() {
		return "PubkeyHashAddress";
	}
};

template <>
struct PythonTypeName<blocksci::script::MultisigPubkey> {
	static std::string name() {
		return "MultisigPubkey";
	}
};

template <>
struct PythonTypeName<blocksci::script::WitnessPubkeyHash> {
	static std::string name() {
		return "WitnessPubkeyHashAddress";
	}
};

template <>
struct PythonTypeName<blocksci::script::Multisig> {
	static std::string name() {
		return "MultisigAddress";
	}
};

template <>
struct PythonTypeName<blocksci::script::Nonstandard> {
	static std::string name() {
		return "NonstandardAddress";
	}
};

template <>
struct PythonTypeName<blocksci::script::OpReturn> {
	static std::string name() {
		return "OpReturn";
	}
};

template <>
struct PythonTypeName<blocksci::script::ScriptHash> {
	static std::string name() {
		return "ScriptHashAddress";
	}
};

template <>
struct PythonTypeName<blocksci::script::WitnessScriptHash> {
	static std::string name() {
		return "WitnessScriptHashAddress";
	}
};

template <>
struct PythonTypeName<blocksci::EquivAddress> {
	static std::string name() {
		return "EquivAddress";
	}
};

template <>
struct PythonTypeName<blocksci::Block> {
	static std::string name() {
		return "Block";
	}
};

template <>
struct PythonTypeName<blocksci::Transaction> {
	static std::string name() {
		return "Tx";
	}
};

template <>
struct PythonTypeName<blocksci::Input> {
	static std::string name() {
		return "Input";
	}
};

template <>
struct PythonTypeName<blocksci::Output> {
	static std::string name() {
		return "Output";
	}
};

template <>
struct PythonTypeName<blocksci::Cluster> {
	static std::string name() {
		return "Cluster";
	}
};

template <>
struct PythonTypeName<blocksci::TaggedCluster> {
	static std::string name() {
		return "TaggedCluster";
	}
};

template <>
struct PythonTypeName<blocksci::TaggedAddress> {
	static std::string name() {
		return "TaggedAddress";
	}
};

template <>
struct PythonTypeName<blocksci::AddressType::Enum> {
	static std::string name() {
		return "address_type";
	}
};

template <>
struct PythonTypeName<blocksci::uint160> {
	static std::string name() {
		return "uint160";
	}
};

template <>
struct PythonTypeName<blocksci::uint256> {
	static std::string name() {
		return "uint256";
	}
};

template <>
struct PythonTypeName<std::array<char, 64>> {
	static std::string name() {
		return "S64";
	}
};

template <>
struct PythonTypeName<std::chrono::system_clock::time_point> {
	static std::string name() {
		return "datetime";
	}
};

template <>
struct PythonTypeName<NumpyDatetime> {
	static std::string name() {
		return "datetime64[ns]";
	}
};

template <>
struct PythonTypeName<int64_t> {
	static std::string name() {
		return "int";
	}
};

template <>
struct PythonTypeName<bool> {
	static std::string name() {
		return "bool";
	}
};

template <>
struct PythonTypeName<NumpyBool> {
	static std::string name() {
		return "bool";
	}
};

template <typename T>
struct PythonTypeName<ranges::optional<T>> {
	static std::string name() {
		return "Optional[" + PythonTypeName<T>::name() + "]";
	}
};

template <typename T>
struct PythonTypeName<ranges::any_view<T, ranges::category::random_access>> {
	static std::string name() {
		return PythonTypeName<T>::name() + "Range";
	}
};

template <typename T>
struct PythonTypeName<ranges::any_view<T>> {
	static std::string name() {
		return PythonTypeName<T>::name() + "Iterator";
	}
};

template <typename T>
struct PythonTypeName<ranges::any_view<ranges::optional<T>, ranges::category::random_access>> {
	static std::string name() {
		return PythonTypeName<T>::name() + "OptionalRange";
	}
};

template <typename T>
struct PythonTypeName<ranges::any_view<ranges::optional<T>>> {
	static std::string name() {
		return PythonTypeName<T>::name() + "OptionalIterator";
	}
};

template <>
struct PythonTypeName<pybind11::list> {
	static std::string name() {
		return "list";
	}
};

template <>
struct PythonTypeName<pybind11::array> {
	static std::string name() {
		return "numpy.ndarray";
	}
};

template <typename T>
struct PythonTypeName<pybind11::array_t<T>> {
	static std::string name() {
		return "numpy.ndarray[" + PythonTypeName<T>::name() + "]";
	}
};

template <typename A, typename B>
struct PythonTypeName<std::pair<A,B>> {
	static std::string name() {
		return "Tuple[" + PythonTypeName<A>::name() + ", " + PythonTypeName<B>::name() + "]";
	}
};

template <>
struct PythonTypeName<pybind11::bytes> {
	static std::string name() {
		return "bytes";
	}
};

template <>
struct PythonTypeName<std::string> {
	static std::string name() {
		return "str";
	}
};

#endif // method_types_h