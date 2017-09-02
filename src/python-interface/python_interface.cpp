//
//  blocksci.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/14/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include <pybind11/pybind11.h>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace py = pybind11;

namespace pybind11 { namespace detail {
    
    template<typename... Ts>
    struct type_caster<boost::variant<Ts...>>
    {
        struct visitor : boost::static_visitor<handle>
        {
            visitor(return_value_policy policy, handle parent)
            : m_policy(policy), m_parent(parent)
            {}
            
            template<typename T>
            handle operator()(T const &t) const
            {
                return type_caster<T>::cast(t, m_policy, m_parent);
            }
            
            return_value_policy m_policy;
            handle              m_parent;
        };
        
        static handle cast(const boost::variant<Ts...> src, return_value_policy policy, handle parent)
        {
            return boost::apply_visitor(visitor(policy, parent), src);
        }
        PYBIND11_TYPE_CASTER(boost::variant<Ts...>, _("Variant"));
    };
    
    template<typename T>
    struct type_caster<boost::optional<T>>
    {
        static handle cast(boost::optional<T> src, return_value_policy policy, handle parent)
        {
            if (src)
            {
                return type_caster<T>::cast(*src, policy, parent);
            }
            
            // if not set convert to None
            return none().inc_ref();
        }
        PYBIND11_TYPE_CASTER(boost::optional<T>, _("Optional"));
    };
    
}} // namespace pybind11::detail

void init_address(py::module &m);
void init_address_index(py::module &m);
void init_tx(py::module &m);
void init_tx_summary(py::module &m);
void init_inout(py::module &m);
void init_block(py::module &m);
void init_blockchain(py::module &m);

PYBIND11_MODULE(blocksci_interface, m) {
    init_address(m);
    init_address_index(m);
    init_blockchain(m);
    init_block(m);
    init_tx(m);
    init_tx_summary(m);
    init_inout(m);
}
