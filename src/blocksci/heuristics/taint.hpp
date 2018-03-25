//
//  taint.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/20/18.
//

#ifndef taint_hpp
#define taint_hpp

#include <blocksci/chain/output.hpp>
#include <stdio.h>
#include <vector>

namespace blocksci { namespace heuristics {
    std::vector<std::pair<Output, uint64_t>> getHaircutTainted(const Output &output, uint64_t taintedValue);
}}

#endif /* taint_hpp */
