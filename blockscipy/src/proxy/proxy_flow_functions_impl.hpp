//
//  proxy_flow_functions_impl.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/20/18.
//

#ifndef proxy_flow_functions_impl_hpp
#define proxy_flow_functions_impl_hpp

#include "proxy.hpp"
#include "proxy_type_check.hpp"
#include "caster_py.hpp"

#include <range/v3/view/concat.hpp>
#include <range/v3/view/join.hpp>

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

template <typename T, typename Range>
RawIterator<T> traverse(Proxy<RawIterator<T>> body, Range && v) {
    return ranges::view::join(std::forward<Range>(v) | ranges::view::transform([body = std::move(body)](auto && item) {
            auto newItems = body(item);
            return ranges::view::concat(ranges::view::single(std::move(item)), traverse(std::move(body), std::move(newItems)));
    }));
}

template <typename T>
void addProxyFlowFunctions(pybind11::module &m, pybind11::module &) {
    m
    .def("take_while", [](Proxy<ranges::optional<T>> &body, Proxy<ranges::optional<T>> &initF) -> Proxy<RawIterator<T>> {
        return {std::function<RawIterator<T>(std::any &)>{[body, initF](std::any &v) -> RawIterator<T> {
            return RawIterator<T>{take_while_range<T>{std::move(body), initF(v)}};
        }}, initF.sourceType};
    })
    .def("take_while", [](Proxy<ranges::optional<T>> &body, ranges::optional<T> &init) -> Iterator<T> {
        return RawIterator<T>{take_while_range<T>{std::move(body), init}};
    })
    .def("traverse", [](Proxy<RawIterator<T>> &body, const T &init) -> Iterator<T> {
        return traverse(body, RawIterator<T>{ranges::view::single(init)});
    })
    ;
}

#endif /* proxy_flow_functions_impl_hpp */
