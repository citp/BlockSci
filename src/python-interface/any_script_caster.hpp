//
//  any_script_caster.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/11/18.
//
//

#ifndef blocksci_any_script_caster_h
#define blocksci_any_script_caster_h

#include "variant_py.hpp"
#include <blocksci/scripts/script_variant.hpp>
#include <pybind11/pybind11.h>

namespace pybind11 { namespace detail {
using blocksci::AnyScript;

template <> struct type_caster<blocksci::AnyScript> {
private:
    using value_conv = make_caster<blocksci::ScriptVariant>;
public:
    bool load(handle src, bool convert) {
        value_conv inner_caster;
        if (!inner_caster.load(src, convert))
            return false;
        value = cast_op<blocksci::ScriptVariant &&>(std::move(inner_caster));
        return true;
    }

    template <typename T>
    static handle cast(T && src, return_value_policy policy, handle parent) {
        return value_conv::cast(std::forward<T>(src).wrapped, policy, parent);
    }

    PYBIND11_TYPE_CASTER(AnyScript, _("AnyAddress"));

    type_caster() = default;
};
}}


#endif /* blocksci_any_script_caster_h */