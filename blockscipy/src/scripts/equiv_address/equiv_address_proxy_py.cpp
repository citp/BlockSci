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

struct AddEquivAddressMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        namespace py = pybind11;
        func(property_tag, "is_script_equiv", &EquivAddress::isScriptEquiv, "Returns whether this equiv address is script equivalent or not");
        func(method_tag, "balance", &EquivAddress::calculateBalance, "Calculates the balance held by these equivalent addresses at the height (Defaults to the full chain)", py::arg("height") = -1);
        func(method_tag, "outs", +[](EquivAddress &address) -> Iterator<Output> {
            return address.getOutputs();
        }, "Returns an iterator over all outputs sent to these equivalent addresses");
        func(method_tag, "ins", +[](EquivAddress &address) -> Iterator<Input> {
            return address.getInputs();
        }, "Returns an iterator over all inputs spent from these equivalent addresses");
        func(method_tag, "out_txes_count", +[](EquivAddress &address) -> int64_t {
            return address.getOutputTransactions().size();
        }, "Return the number of transactions where these equivalent addresses were an output");
        func(method_tag, "in_txes_count", +[](EquivAddress &address) -> int64_t {
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
