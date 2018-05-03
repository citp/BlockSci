//
//  caster_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef caster_py_hpp
#define caster_py_hpp

#include <blocksci/address/address.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <range/v3/utility/optional.hpp>
#include <mpark/variant.hpp>

namespace pybind11 { namespace detail {
    // Specifies the function used to visit the variant -- `apply_visitor` instead of `visit`
    template <>
    struct visit_helper<mpark::variant> {
        template <typename... Args>
        static auto call(Args &&...args) -> decltype(mpark::visit(args...)) {
            return mpark::visit(args...);
        }
    };

    template <typename... Ts>
    struct type_caster<mpark::variant<Ts...>> : variant_caster<mpark::variant<Ts...>> {};

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
        
        static handle cast(const blocksci::AnyScript &src, return_value_policy policy, handle parent) {
            return value_conv::cast(src.wrapped, policy, parent);
        }
        
        PYBIND11_TYPE_CASTER(blocksci::AnyScript, _("AnyAddress"));
        
        type_caster() = default;
    };

    template <> struct type_caster<blocksci::Address> {
    private:
        using value_conv = make_caster<blocksci::AnyScript>;
    public:
        bool load(handle src, bool convert) {
            value_conv inner_caster;
            if (!inner_caster.load(src, convert))
                return false;

            auto any = cast_op<blocksci::AnyScript &&>(std::move(inner_caster));
            mpark::visit([&](auto address) {
                value = address;
            }, any.wrapped);
            return true;
        }
        
        static handle cast(const blocksci::Address &src, return_value_policy policy, handle parent) {
            return value_conv::cast(src.getScript(), policy, parent);
        }

        PYBIND11_TYPE_CASTER(blocksci::Address, _("Address"));
        
        type_caster() = default;
    };
    
    template <typename T>
    struct type_caster<ranges::optional<T>> : optional_caster<ranges::optional<T>> {};
}}

#endif /* caster_py_hpp */
