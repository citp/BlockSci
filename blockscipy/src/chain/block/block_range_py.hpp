//
//  block_range_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef block_range_py_h
#define block_range_py_h

#include "python_fwd.hpp"
#include "method_types.hpp"

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/block_range.hpp>
#include <blocksci/cluster/cluster.hpp>

#include <pybind11/pybind11.h>

#include <range/v3/view/any_view.hpp>


struct BlockRangeClasses {
    pybind11::class_<ranges::any_view<blocksci::Block>> iterator;
    pybind11::class_<blocksci::BlockRange> range;
    pybind11::class_<ranges::any_view<ranges::optional<blocksci::Block>>> optionalIterator;
    pybind11::class_<ranges::any_view<ranges::optional<blocksci::Block>, ranges::category::random_access | ranges::category::sized>> optionalRange;

    BlockRangeClasses(pybind11::module &m) : 
    iterator(m, strdup(PythonTypeName<ranges::any_view<blocksci::Block>>::name().c_str())),
    range(m, strdup(PythonTypeName<blocksci::BlockRange>::name().c_str())),
    optionalIterator(m, strdup(PythonTypeName<ranges::any_view<ranges::optional<blocksci::Block>>>::name().c_str())),
    optionalRange(m, strdup(PythonTypeName<ranges::any_view<ranges::optional<blocksci::Block>, ranges::category::random_access | ranges::category::sized>>::name().c_str())) {}
};


void addBlockRangeMethods(BlockRangeClasses &classes);
void applyMethodsToBlockRange(BlockRangeClasses &classes);

#endif /* block_py_h */
