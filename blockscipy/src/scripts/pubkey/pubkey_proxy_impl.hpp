//
//  pubkey_proxy_impl.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_pubkey_proxy_impl_h
#define blocksci_pubkey_proxy_impl_h

#include "python_fwd.hpp"
#include "method_tags.hpp"

#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/scripts/bitcoin_pubkey.hpp>

#include <pybind11/pybind11.h>

#include <range/v3/utility/optional.hpp>
#include <range/v3/view/transform.hpp>

template <typename T>
struct AddPubkeyBaseMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        func(method_tag, "find_multisigs", +[](const T &script) -> RawIterator<script::Multisig> {
            return script.getIncludingMultisigs() | ranges::views::transform([](Address && address) -> script::Multisig {
                return mpark::get<script::Multisig>(address.getScript().wrapped);
            });
        }, "List of multisigs which include this public key");
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

#endif /* blocksci_pubkey_proxy_impl_h */
