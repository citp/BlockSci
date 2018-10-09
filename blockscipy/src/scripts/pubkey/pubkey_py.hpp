//
//  pubkey_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_pubkey_py_h
#define blocksci_pubkey_py_h

#include "method_tags.hpp"
#include "blocksci_range.hpp"

#include <blocksci/scripts/multisig_script.hpp>
#include <blocksci/scripts/bitcoin_pubkey.hpp>

#include <pybind11/pybind11.h>

#include <range/v3/utility/optional.hpp>

template <typename T>
struct AddPubkeyBaseMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(method_tag, "find_multisigs", &T::getIncludingMultisigs, "List of multisigs which include this public key");
        func(property_tag, "pubkey", +[](const T &script) -> ranges::optional<pybind11::bytes> {
            auto pubkey = script.getPubkey();
            if (pubkey) {
                return pybind11::bytes(reinterpret_cast<const char *>(pubkey->begin()), pubkey->size());
            } else {
                return ranges::nullopt;
            }
        }, "Public key for this address");
        func(property_tag, "pubkeyhash", &T::getPubkeyHash, "160 bit address hash");
        func(property_tag, "address_string", &T::addressString, "Bitcoin address string");
    }
};

void init_pubkey(pybind11::class_<blocksci::script::Pubkey> &cl);
void init_pubkeyhash(pybind11::class_<blocksci::script::PubkeyHash> &cl);
void init_witness_pubkeyhash(pybind11::class_<blocksci::script::WitnessPubkeyHash> &cl);
void init_multisig_pubkey(pybind11::class_<blocksci::script::MultisigPubkey> &cl);

void addMultisigPubkeyRangeMethods(RangeClasses<blocksci::script::MultisigPubkey> &classes);
void addPubkeyRangeMethods(RangeClasses<blocksci::script::Pubkey> &classes);
void addPubkeyHashRangeMethods(RangeClasses<blocksci::script::PubkeyHash> &classes);
void addWitnessPubkeyHashRangeMethods(RangeClasses<blocksci::script::WitnessPubkeyHash> &classes);

#endif /* blocksci_pubkey_py_h */
