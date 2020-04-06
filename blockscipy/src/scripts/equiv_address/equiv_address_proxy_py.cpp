//
//  equiv_address_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/22/18.
//
//

#include "equiv_address_proxy_py.hpp"
#include "equiv_address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/cluster/cluster.hpp>
#include <blocksci/address/equiv_address.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/join.hpp>

struct AddEquivAddressMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        namespace py = pybind11;
        func(property_tag, "is_script_equiv", &EquivAddress::isScriptEquiv, "Returns whether this equiv address is script equivalent or not");
        func(method_tag, "balance", &EquivAddress::calculateBalance, "Calculates the balance held by these equivalent addresses at the height (Defaults to the full chain)", py::arg("height") = -1);
        func(property_tag, "addresses", +[](const EquivAddress &address) -> RawIterator<AnyScript> {
            return ranges::views::ints(0, 1) | ranges::views::transform([address](int) {
                return address | ranges::views::transform([](const Address &address) {
                    return address.getScript();
                });
            }) | ranges::views::join;
        }, "Calculate balance");

        func(property_tag, "outputs", +[](const EquivAddress &address) -> RawIterator<Output> {
            return ranges::views::ints(0, 1) | ranges::views::transform([address](int) {
                return address.getOutputs();
            }) | ranges::views::join;
        }, "Returns an iterator over all outputs sent to these equivalent addresses");
        func(property_tag, "outs", +[](const EquivAddress &address) -> RawIterator<Output> {
            pybind11::print("Warning: `outs` is deprecated. Use `outputs` instead.");
            return ranges::views::ints(0, 1) | ranges::views::transform([address](int) {
                return address.getOutputs();
            }) | ranges::views::join;
        }, "Returns an iterator over all outputs sent to these equivalent addresses");

        func(property_tag, "inputs", +[](EquivAddress &address) -> RawIterator<Input> {
            return ranges::views::ints(0, 1) | ranges::views::transform([address](int) {
                return address.getInputs();
            }) | ranges::views::join;
        }, "Returns an iterator over all inputs spent from these equivalent addresses");
        func(property_tag, "ins", +[](EquivAddress &address) -> RawIterator<Input> {
            pybind11::print("Warning: `ins` is deprecated. Use `inputs` instead.");
            return ranges::views::ints(0, 1) | ranges::views::transform([address](int) {
                return address.getInputs();
            }) | ranges::views::join;
        }, "Returns an iterator over all inputs spent from these equivalent addresses");

        func(method_tag, "output_txes_count", +[](EquivAddress &address) -> int64_t {
            return address.getOutputTransactions().size();
        }, "Return the number of transactions where these equivalent addresses were an output");
        func(method_tag, "out_txes_count", +[](EquivAddress &address) -> int64_t {
            pybind11::print("Warning: `out_txes_count` is deprecated. Use `output_txes_count` instead.");
            return address.getOutputTransactions().size();
        }, "Return the number of transactions where these equivalent addresses were an output");

        func(method_tag, "input_txes_count", +[](EquivAddress &address) -> int64_t {
            return address.getInputTransactions().size();
        }, "Return the number of transactions where these equivalent addresses were an input");
        func(method_tag, "in_txes_count", +[](EquivAddress &address) -> int64_t {
            pybind11::print("Warning: `in_txes_count` is deprecated. Use `input_txes_count` instead.");
            return address.getInputTransactions().size();
        }, "Return the number of transactions where these equivalent addresses were an input");
    }
};

void addEquivAddressProxyMethods(AllProxyClasses<blocksci::EquivAddress> &cls) {
	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

    applyMethodsToProxy(cls.base, AddEquivAddressMethods{});
    addProxyEqualityMethods(cls.base);
}
