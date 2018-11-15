//
//  method_types.cpp
//  blocksci
//
//  Created by Harry Kalodner on 4/30/18.
//
//

#include "method_types.hpp"
#include "python_fwd.hpp"
#include "generic_sequence.hpp"

#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <range/v3/utility/optional.hpp>

#include <chrono>

using namespace blocksci;
namespace py = pybind11;

template <typename T>
void addTypeName(
    std::unordered_map<std::type_index, std::string> &type_names,
    std::unordered_map<std::type_index, std::string> &docstring_type_names,
    const std::string &name,
    bool isBlocksci = true) {
    type_names[std::type_index(typeid(T))] = name;
    type_names[std::type_index(typeid(ranges::optional<T>))] = "Optional" + name;
    type_names[std::type_index(typeid(Sequence<T>))] = name + "Sequence";
    type_names[std::type_index(typeid(Iterator<T>))] = name + "Iterator";
    type_names[std::type_index(typeid(Range<T>))] = name + "Range";
    type_names[std::type_index(typeid(RawIterator<T>))] = name + "Iterator";
    type_names[std::type_index(typeid(RawRange<T>))] = name + "Range";

    if (isBlocksci) {
        docstring_type_names[std::type_index(typeid(T))] = "blocksci." + name;
        docstring_type_names[std::type_index(typeid(ranges::optional<T>))] = "Optional[blocksci." + name + "]";
        docstring_type_names[std::type_index(typeid(Iterator<T>))] = "blocksci." + name + "Iterator";
        docstring_type_names[std::type_index(typeid(Range<T>))] = "blocksci." + name + "Range";
        docstring_type_names[std::type_index(typeid(RawIterator<T>))] = "blocksci." + name + "Iterator";
        docstring_type_names[std::type_index(typeid(RawRange<T>))] = "blocksci." + name + "Range";
    } else {
        docstring_type_names[std::type_index(typeid(ranges::optional<T>))] = "Optional[" + name + "]";
    }
    docstring_type_names[std::type_index(typeid(pybind11::array_t<T>))] = "numpy.ndarray[" + name + "]";
}

TypenameLookup::TypenameLookup() {
    addTypeName<AnyScript>(typeNames, docstringTypeNames, "Address");
    addTypeName<Address>(typeNames, docstringTypeNames, "Address");
    addTypeName<script::Pubkey>(typeNames, docstringTypeNames, "PubkeyAddress");
    addTypeName<script::PubkeyHash>(typeNames, docstringTypeNames, "PubkeyHashAddress");
    addTypeName<script::MultisigPubkey>(typeNames, docstringTypeNames, "MultisigPubkey");
    addTypeName<script::WitnessPubkeyHash>(typeNames, docstringTypeNames, "WitnessPubkeyHashAddress");
    addTypeName<script::Multisig>(typeNames, docstringTypeNames, "MultisigAddress");
    addTypeName<script::Nonstandard>(typeNames, docstringTypeNames, "NonstandardAddress");
    addTypeName<script::OpReturn>(typeNames, docstringTypeNames, "OpReturn");
    addTypeName<script::ScriptHash>(typeNames, docstringTypeNames, "ScriptHashAddress");
    addTypeName<script::WitnessScriptHash>(typeNames, docstringTypeNames, "WitnessScriptHashAddress");
    addTypeName<script::WitnessUnknown>(typeNames, docstringTypeNames, "WitnessUnknownAddress");

    addTypeName<EquivAddress>(typeNames, docstringTypeNames, "EquivAddress");
    addTypeName<Block>(typeNames, docstringTypeNames, "Block");
    addTypeName<Transaction>(typeNames, docstringTypeNames, "Tx");
    addTypeName<Input>(typeNames, docstringTypeNames, "Input");
    addTypeName<Output>(typeNames, docstringTypeNames, "Output");
    addTypeName<Cluster>(typeNames, docstringTypeNames, "Cluster");
    addTypeName<TaggedCluster>(typeNames, docstringTypeNames, "TaggedCluster");
    addTypeName<TaggedAddress>(typeNames, docstringTypeNames, "TaggedAddress");

    addTypeName<AddressType::Enum>(typeNames, docstringTypeNames, "address_type");
    addTypeName<uint160>(typeNames, docstringTypeNames, "uint160");
    addTypeName<uint256>(typeNames, docstringTypeNames, "uint256");
    addTypeName<std::array<char, 40>>(typeNames, docstringTypeNames, "S40", false);
    addTypeName<std::array<char, 64>>(typeNames, docstringTypeNames, "S64", false);
    addTypeName<std::chrono::system_clock::time_point>(typeNames, docstringTypeNames, "datetime.datetime", false);
    addTypeName<NumpyDatetime>(typeNames, docstringTypeNames, "datetime64[ns]", false);
    addTypeName<int64_t>(typeNames, docstringTypeNames, "int", false);
    addTypeName<bool>(typeNames, docstringTypeNames, "bool", false);
    addTypeName<NumpyBool>(typeNames, docstringTypeNames, "bool", false);
    addTypeName<py::list>(typeNames, docstringTypeNames, "list", false);
    addTypeName<py::array>(typeNames, docstringTypeNames, "numpy.ndarray", false);
    addTypeName<py::bytes>(typeNames, docstringTypeNames, "bytes", false);
    addTypeName<std::string>(typeNames, docstringTypeNames, "str", false);
}

const TypenameLookup &typenameLookup() {
    static TypenameLookup typenameLookup;
    return typenameLookup;
}

std::string getTypeName(const char *text, const std::type_info *const *types) {
    std::string signature;
    size_t type_depth = 0, char_index = 0, type_index = 0;
    while (true) {
        char c = text[char_index++];
        if (c == '\0')
            break;

        if (c == '{') {
            ++type_depth;
        } else if (c == '}') {
            --type_depth;
        } else if (c == '%') {
            const std::type_info *t = types[type_index++];
            if (!t)
                pybind11::pybind11_fail("Internal error while parsing type signature (1)");
            if (auto tinfo = pybind11::detail::get_type_info(*t)) {
                pybind11::handle th((PyObject *) tinfo->type);
                signature +=
                    th.attr("__module__").cast<std::string>() + "." +
                    th.attr("__qualname__").cast<std::string>(); // Python 3.3+, but we backport it to earlier versions
            } else {
                std::string tname(t->name());
                pybind11::detail::clean_type_id(tname);
                signature += tname;
            }
        } else {
            signature += c;
        }
    }
    return signature;
}
