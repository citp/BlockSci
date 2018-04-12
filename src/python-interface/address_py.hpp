//
//  address_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef blocksci_address_py_h
#define blocksci_address_py_h

#include "ranges_py.hpp"
#include "variant_py.hpp"

#include <blocksci/chain/transaction.hpp>
#include <blocksci/scripts/script.hpp>
#include <blocksci/address/equiv_address.hpp>
#include <pybind11/pybind11.h>

pybind11::class_<blocksci::ScriptBase> init_address(pybind11::module &m);

template <typename T, typename Class, typename FuncApplication, typename FuncDoc>
void addAddressMethods(Class &cl, FuncApplication func, FuncDoc func2) {
	using namespace blocksci;
    cl
    .def_property_readonly("address_num", func([](const T &address) -> int64_t {
    	return address.getScriptNum();
    }), func2("The internal identifier of the address"))
    .def_property_readonly("type", func([](const T &address) -> AddressType::Enum {
    	return address.getType();
    }), func2("The type of address"))
    .def("equiv", func([](const T &address, bool equiv_script) -> EquivAddress {
    	return address.getEquivAddresses(equiv_script);
    }), pybind11::arg("equiv_script") = true, "Returns a list of all addresses equivalent to this address")
    .def("balance", func([](const T &address, int height) -> int64_t {
    	return address.calculateBalance(height);
    }), pybind11::arg("height") = -1, func2("Calculates the balance held by this address at the height (Defaults to the full chain)"))
    .def("out_txes_count", func([](const T &address) -> int64_t {
        return address.getOutputTransactions().size();
    }), func2("Return the number of transactions where this address was an output"))
    .def("in_txes_count", func([](const T &address) -> int64_t {
        return address.getInputTransactions().size();
    }), func2("Return the number of transactions where this address was an input"))
    .def_property_readonly("first_tx", func([](const T &address) -> Transaction {
    	return address.getFirstTransaction();
    }), func2("Get the first transaction that was sent to a type equivalent address"))
    .def_property_readonly("revealed_tx", func([](const T &address) -> ranges::optional<Transaction> {
    	return address.getTransactionRevealed();
    }), func2("The transaction where a type equivalent address was first revealed"))
    .def_property_readonly("has_been_spent", func([](const T &address) -> bool {
    	return address.hasBeenSpent();
    }), func2("Check if a type equivalent address has ever been spent"))
    ;
}

template <blocksci::AddressType::Enum type>
using ScriptRange = ranges::any_view<blocksci::ScriptAddress<type>>;
using ScriptRangeVariant = blocksci::to_variadic_t<blocksci::to_address_tuple_t<ScriptRange>, mpark::variant>;

template<blocksci::AddressType::Enum type, typename Range>
struct ScriptRangeWithTypeFunctor {
    static ScriptRangeVariant f(Range &&range) {
        return range |
        ranges::view::filter([](const auto &address) { return address.getType() == type; }) |
        ranges::view::transform([](const auto &address) { return mpark::get<blocksci::ScriptAddress<type>>(address.wrapped); });
    }
};

template <typename Range, typename Class, typename FuncApplication>
struct AddAddressRangeMethods {
	void operator()(Class &cl, FuncApplication func) {
	    cl
	    .def("with_type", func([=](Range &&range, blocksci::AddressType::Enum type) {
	    	static auto table = blocksci::make_dynamic_table<blocksci::AddressType, ScriptRangeWithTypeFunctor, Range>();
	        auto index = static_cast<size_t>(type);
	        return table.at(index)(std::move(range));
	    }), "Returns a range including the subset of addresses which have the given type")
	    ;
	}
};


#endif /* blocksci_address_py_h */