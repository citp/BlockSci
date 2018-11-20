//
//  proxy_functions_impl.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/17/18.
//

#ifndef proxy_functions_impl_hpp
#define proxy_functions_impl_hpp

#include "proxy.hpp"
#include "proxy_type_check.hpp"
#include "caster_py.hpp"

template <typename T>
void addProxyFunctionsT(pybind11::module &m) {
    m
    .def("conditional", [](const Proxy<bool> &cond, const Proxy<T> &p1, const Proxy<T> &p2) -> Proxy<T> {
        cond.sourceType.checkMatch(p1.sourceType);
        cond.sourceType.checkMatch(p2.sourceType);
        return {std::function<T(std::any &)>{[cond, p1, p2](std::any &t) -> T {
            if(cond(t)) {
                return p1(t);
            } else {
                return p2(t);
            }
        }}, p1.sourceType};
    })
    .def("while_loop", [](const Proxy<bool> &cond, const Proxy<T> &body) -> Proxy<T> {
        cond.sourceType.checkMatch(body.sourceType);
        cond.sourceType.checkAccept(body.getDestType());

        return {std::function<T(std::any &)>{[cond, body](std::any &t) -> T {
            std::any v = t;
            while(cond(v)) {
                v = body(v);
            }
            return std::any_cast<T>(v);
        }}, cond.sourceType};
    })
    ;
}

template <typename T>
void addProxyFunctions(pybind11::module &, pybind11::module &pm) {
    addProxyFunctionsT<T>(pm);
    addProxyFunctionsT<ranges::optional<T>>(pm);
    addProxyFunctionsT<RawIterator<T>>(pm);
    addProxyFunctionsT<RawRange<T>>(pm);

    pm
    .def("conditional", [](const Proxy<bool> &cond, const Proxy<T> &p) -> Proxy<ranges::optional<T>> {
        cond.sourceType.checkMatch(p.sourceType);
        return {std::function<ranges::optional<T>(std::any &)>{[cond, p](std::any &t) -> ranges::optional<T> {
            if(cond(t)) {
                return p(t);
            } else {
                return ranges::nullopt;
            }
        }}, p.sourceType};
    })
    ;
}

#endif /* proxy_functions_impl_hpp */
