//
//  proxy_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/19/18.
//

#ifndef proxy_py_hpp
#define proxy_py_hpp

#include "sequence.hpp"
#include "python_fwd.hpp"

#include <pybind11/pybind11.h>

#include <range/v3/utility/optional.hpp>

template <typename T, typename SimpleBase>
struct AllProxyClasses {
    using SimplePy = pybind11::class_<Proxy<T>, SimpleBase>;
    using OptionalPy = pybind11::class_<Proxy<ranges::optional<T>>, OptionalProxy>;
    using SequencePy = pybind11::class_<SequenceProxy<T>>;
    using IteratorPy = pybind11::class_<Proxy<RawIterator<T>>, IteratorProxy, SequenceProxy<T>>;
    using RangePy = pybind11::class_<Proxy<RawRange<T>>, RangeProxy, SequenceProxy<T>>;

    SimplePy base;
    OptionalPy optional;
    SequencePy sequence;
    IteratorPy iterator;
    RangePy range;

	template<typename Func>
    void applyToAll(Func func) {
    	func(base);
        func(optional);
        func(iterator);
        func(range);
    }
};

#endif /* proxy_py_hpp */


