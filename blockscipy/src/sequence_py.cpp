//
//  sequence_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/11/18.
//

#include "sequence_py.hpp"
#include "generic_sequence.hpp"
#include "generic_proxy.hpp"

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

void addCommonIteratorMethods(pybind11::class_<GenericIterator> &cl) {
    cl
    .def("_group_by", [](GenericIterator &range, SimpleProxy &grouper, SimpleProxy &eval) -> pybind11::dict {
        std::unordered_map<BlocksciType, std::vector<BlocksciType>> grouped;
        auto genericGrouper = grouper.getGenericSimple();
        RANGES_FOR(auto && item, range.getGenericIterator()) {
            auto anyItem = item.toAny();
            auto group = genericGrouper(anyItem);
            grouped[group].push_back(item);
        }
        pybind11::dict results;
        auto genericEval = eval.getGenericSimple();
        for (auto &group : grouped) {
            std::any range = RawRange<BlocksciType>{group.second};
            results[group.first.toObject()] = genericEval(range).toObject();
        }
        return results;
    })
    .def("to_list", [](GenericIterator & range) { 
        pybind11::list list;
        RANGES_FOR(auto && item, range.getGenericIterator()) {
        	addToList(list, item);
        }
        return list;
    }, "Returns a list of all of the objects in the range");
}
