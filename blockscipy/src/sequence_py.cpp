//
//  sequence_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/11/18.
//

#include "sequence_py.hpp"
#include "generic_sequence.hpp"
#include "generic_proxy.hpp"
#include "caster_py.hpp"
#include "blocksci_type.hpp"
#include "blocksci_iterator_type.hpp"

void addCommonRangeMethods(pybind11::class_<GenericRange, GenericIterator> &cl) {
    cl
    .def("__bool__", [](GenericRange &range) {
        return !range.empty();
        
    })
    .def("__len__", [](GenericRange &range) {
        return range.size();
    })
    ;
}

struct GroupByVisitor {
    SimpleProxy &grouper;
    SimpleProxy &eval;
    
    template <typename T>
    pybind11::dict operator()(RawIterator<T> && rng) const {
        std::unordered_map<BlocksciType, std::vector<T>> grouped;
        auto genericGrouper = grouper.getGenericSimple();
        RANGES_FOR(auto item, rng) {
            std::any anyItem = item;
            auto group = genericGrouper(anyItem);
            grouped[group].emplace_back(std::move(item));
        }
        pybind11::dict results;
        auto genericEval = eval.getGenericSimple();
        for (auto &group : grouped) {
            std::any range = RawRange<T>{group.second};
            results[group.first.toObject()] = genericEval(range).toObject();
        }
        return results;
    }
};

struct ToListVisitor {
    template <typename T>
    pybind11::list operator()(RawIterator<T> && rng) const {
        pybind11::list list;
        RANGES_FOR(auto && item, rng) {
            list.append(std::forward<decltype(item)>(item));
        }
        return list;
    }
};

void addCommonIteratorMethods(pybind11::class_<GenericIterator> &cl) {
    cl
    .def("_group_by", [](GenericIterator &range, SimpleProxy &grouper, SimpleProxy &eval) -> pybind11::dict {
        return mpark::visit(GroupByVisitor{grouper, eval}, range.getGenericIterator().var);
    })
    .def("to_list", [](GenericIterator &range) { 
        return mpark::visit(ToListVisitor{}, range.getGenericIterator().var);
    }, "Returns a list of all of the objects in the range");
}
