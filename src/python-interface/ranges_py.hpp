//
//  ranges_py.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/19/17.
//

#ifndef ranges_py_hpp
#define ranges_py_hpp

#include <blocksci/chain/block.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/slice.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

template<typename Range, CONCEPT_REQUIRES_(ranges::RandomAccessRange<Range>() && ranges::SizedRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name) {
    pybind11::class_<Range> cl(m, name.c_str());
    cl
    .def("__len__", [](Range &chain) { return chain.size(); })
    .def("__iter__", [](Range &chain) { return pybind11::make_iterator(chain.begin(), chain.end()); },
         pybind11::keep_alive<0, 1>())
    .def("__getitem__", [](Range &chain, ranges::range_difference_type_t<Range> i) {
        auto chainSize = chain.size();
        if (i < 0) {
            i += chainSize;
        }
        uint64_t posIndex = static_cast<uint64_t>(i);
        if (posIndex >= chainSize) {
            throw pybind11::index_error();
        }
        return chain[i];
    })
    .def("__getitem__", [](Range &chain, pybind11::slice slice) -> ranges::any_view<decltype(chain[0])> {
        size_t start, stop, step, slicelength;
        if (!slice.compute(chain.size(), &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();

        auto subset =  ranges::view::slice(chain, 
            static_cast<ranges::range_difference_type_t<Range>>(start), 
            static_cast<ranges::range_difference_type_t<Range>>(stop));
        auto strided = subset | ranges::view::stride(step);
        return strided;
    })
    ;
    return cl;
}

template<typename Range, CONCEPT_REQUIRES_(ranges::RandomAccessRange<Range>() && !ranges::SizedRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name) {
    pybind11::class_<Range> cl(m, name.c_str());
    cl
    .def("__len__", [](Range &chain) { return ranges::distance(chain); })
    .def("__iter__", [](Range &chain) { return pybind11::make_iterator(chain.begin(), chain.end()); },
         pybind11::keep_alive<0, 1>())
    .def("__getitem__", [](Range &chain, int64_t i) {
        if (i < 0) {
            auto chainSize = ranges::distance(chain);
            i = (chainSize + i) % chainSize;
        }
        uint64_t posIndex = static_cast<uint64_t>(i);
        if (posIndex >= ranges::distance(chain)) {
            throw pybind11::index_error();
        }
        return chain[i];;
    })
    .def("__getitem__", [](Range &chain, pybind11::slice slice) -> ranges::any_view<decltype(chain.front())> {
        size_t start, stop, step, slicelength;
        if (!slice.compute(ranges::distance(chain), &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();
        auto subset =  ranges::view::slice(chain, start, stop);
        auto strided = subset | ranges::view::stride(step);
        return strided;
    })
    ;
    return cl;
}

template<typename Range, CONCEPT_REQUIRES_(ranges::BidirectionalRange<Range>() && !ranges::RandomAccessRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name) {
    pybind11::class_<Range> cl(m, name.c_str());
    cl
    .def("__len__", [](Range &range) { return ranges::distance(range); })
    .def("__iter__", [](Range &range) { return pybind11::make_iterator(range.begin(), range.end()); },
         pybind11::keep_alive<0, 1>())
    .def("__getitem__", [](Range &range, int64_t i) {
        auto chainSize = ranges::distance(range);
        if (i < 0) {
            i = (chainSize + i) % chainSize;
        }
        uint64_t posIndex = static_cast<uint64_t>(i);
        if (posIndex >= chainSize) {
            throw pybind11::index_error();
        }
        auto it = range.begin();
        ranges::advance(it, i);
        return *it;
    })
    .def("__getitem__", [](Range &range, pybind11::slice slice) -> ranges::any_view<decltype(*range.begin())> {
        size_t start, stop, step, slicelength;
        auto chainSize = ranges::distance(range);
        if (!slice.compute(chainSize, &start, &stop, &step, &slicelength))
            throw pybind11::error_already_set();
        auto subset =  ranges::view::slice(range, start, stop);
        auto strided = subset | ranges::view::stride(step);
        return strided;
    })
    ;
    return cl;
}

template<typename Range, CONCEPT_REQUIRES_(ranges::InputRange<Range>() && !ranges::ForwardRange<Range>())>
auto addRangeClass(pybind11::module &m, const std::string &name) {
    pybind11::class_<Range> cl(m, name.c_str());
    cl
    .def("__iter__", [](Range &range) { return pybind11::make_iterator(range.begin(), range.end()); },
         pybind11::keep_alive<0, 1>())
    ;
    return cl;
}

#endif /* ranges_py_hpp */
