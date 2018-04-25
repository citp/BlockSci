//
//  address_range_py.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "address_py.hpp"
#include "caster_py.hpp"
#include "range_apply_py.hpp"
#include "ranges_py.hpp"

namespace py = pybind11;

using namespace blocksci;

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

template <typename Range>
struct AddAddressRangeMethods {
    template <typename Class, typename FuncApplication>
    void operator()(Class &cl, FuncApplication func) {
        cl
        .def("with_type", func([](Range &range, blocksci::AddressType::Enum type) {
            static auto table = blocksci::make_dynamic_table<blocksci::AddressType, ScriptRangeWithTypeFunctor, Range>();
            auto index = static_cast<size_t>(type);
            return table.at(index)(std::move(range));
        }), "Returns a range including the subset of addresses which have the given type")
        ;
    }
};

const char *addressRangeDocstring(std::string docstring) {
    std::stringstream ss;
    ss << "For each address: " << docstring;
    return strdup(ss.str().c_str());
}


void init_address_range(py::module &m) {
    {
        auto cl = addRangeClass<ranges::any_view<AnyScript>>(m, "AnyAddressRange");
        applyMethodsToRange(cl, AddAddressMethods<AnyScript>{addressRangeDocstring});
        applyRangeMethodsToRange(cl, AddAddressRangeMethods<ranges::any_view<AnyScript>>{});
    }

    {
        auto cl = addRangeClass<ranges::any_view<AnyScript, ranges::category::random_access>>(m, "AddressRange");
        applyMethodsToRange(cl, AddAddressMethods<AnyScript>{addressRangeDocstring});
        applyRangeMethodsToRange(cl, AddAddressRangeMethods<ranges::any_view<AnyScript, ranges::category::random_access>>{});
    }

    {
        auto cl = addRangeClass<ranges::any_view<ranges::optional<AnyScript>>>(m, "AnyOptionalAddressRange");
        applyMethodsToRange(cl, AddAddressMethods<AnyScript>{addressRangeDocstring});
    }

    {
        auto cl = addRangeClass<ranges::any_view<ranges::optional<AnyScript>, ranges::category::random_access>>(m, "OptionalAddressRange");
        applyMethodsToRange(cl, AddAddressMethods<AnyScript>{addressRangeDocstring});
    }
}
