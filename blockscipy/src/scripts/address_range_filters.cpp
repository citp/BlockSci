//
//  address_range_filters.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "address_range_py.hpp"
#include "blocksci_range.hpp"
#include "caster_py.hpp"
#include "range_filter_apply_py.hpp"

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
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        func(method_tag, "with_type", [](Range &range, blocksci::AddressType::Enum type) {
            static auto table = blocksci::make_dynamic_table<blocksci::AddressType, ScriptRangeWithTypeFunctor, Range>();
            auto index = static_cast<size_t>(type);
            return table.at(index)(std::move(range));
        }, "Returns a range including the subset of addresses which have the given type");
    }
};

void applyRangeFiltersToAddressRange(RangeClasses<AnyScript> &classes) {
    applyRangeFiltersToRange<AddAddressRangeMethods>(classes);
}
