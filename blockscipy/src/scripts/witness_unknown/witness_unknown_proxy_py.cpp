//
//  witness_unknown_proxy_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 10/22/18.
//
//

#include "witness_unknown_proxy_py.hpp"
#include "witness_unknown_py.hpp"
#include "scripts/address_py.hpp"
#include "proxy_apply_py.hpp"
#include "proxy/basic.hpp"
#include "proxy/equality.hpp"
#include "proxy/optional.hpp"
#include "proxy/range.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/cluster/cluster.hpp>
#include <blocksci/scripts/witness_unknown_script.hpp>

#include <range/v3/range_for.hpp>

using namespace blocksci;
namespace py = pybind11;

struct AddWitnessUnknownMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(property_tag, "witness_version", +[](const script::WitnessUnknown &script) -> int64_t { return script.witnessVersion(); }, "Witness version of the unknown script");
        func(property_tag, "witness_script", &script::WitnessUnknown::getWitnessScriptString, "Witness output script");
        func(property_tag, "witness_stack", +[](const script::WitnessUnknown &script) -> ranges::optional<py::list> {
            auto stack = script.getWitnessStack();
            if (stack) {
                py::list list;
                RANGES_FOR(auto && item, *stack) {
                    auto charVector = item | ranges::views::transform([](auto && c) -> char { return c; } ) | ranges::to_vector;
                    list.append(py::bytes(std::string{charVector.begin(), charVector.end()}));
                }
                return list;
            } else {
                return ranges::nullopt;
            }
        }, "Witness stack of spending input");
    }
};

void addWitnessUnknownProxyMethods(AllProxyClasses<script::WitnessUnknown, ProxyAddress> &cls) {
	cls.applyToAll(AddProxyMethods{});
    setupRangesProxy(cls);
    addProxyOptionalMethods(cls.optional);

	applyMethodsToProxy(cls.base, AddWitnessUnknownMethods{});
    addProxyEqualityMethods(cls.base);
}
