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
class take_while_range : public ranges::view_facade<take_while_range<T>> {
    friend ranges::range_access;

    ranges::optional<T> v;
    Proxy<ranges::optional<T>> body;

    const T &read() const { 
        return *v;
    }

    bool equal(ranges::default_sentinel) const {
        return !v;
    }

    void next() { 
        v = body(*v); 
    }

public:
    take_while_range() : body(std::function<ranges::optional<T>(std::any &)>{}, createProxyTypeInfo<ranges::optional<T>>()) {};
    explicit take_while_range(
        const Proxy<ranges::optional<T>> &bodyFunc,
        const ranges::optional<T> &initialVal
    ) : v(initialVal), body(bodyFunc) {}
};

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
void addProxyFunctions(pybind11::module &m, pybind11::module &pm) {
    addProxyFunctionsT<T>(pm);
    addProxyFunctionsT<ranges::optional<T>>(pm);
    addProxyFunctionsT<RawIterator<T>>(pm);
    addProxyFunctionsT<RawRange<T>>(pm);

    m
    .def("take_while", [](Proxy<ranges::optional<T>> &body, Proxy<ranges::optional<T>> &initF) -> Proxy<RawIterator<T>> {
        return {std::function<RawIterator<T>(std::any &)>{[body, initF](std::any &v) -> RawIterator<T> {
            return RawIterator<T>{take_while_range<T>{std::move(body), initF(v)}};
        }}, initF.sourceType};
        
    })
    .def("take_while", [](Proxy<ranges::optional<T>> &body, ranges::optional<T> &init) -> Iterator<T> {
        return RawIterator<T>{take_while_range<T>{std::move(body), init}};
    })
    ;
}

#endif /* proxy_functions_impl_hpp */
