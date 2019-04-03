//
//  taint.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/20/18.
//

#ifndef taint_hpp
#define taint_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/core/typedefs.hpp>
#include <blocksci/chain/chain_fwd.hpp>

#include <cstdint>
#include <vector>

namespace blocksci { namespace heuristics {
    using SimpleTaint = std::pair<int64_t, int64_t>; // (tainted value, untainted value)
    using ComplexTaint = std::vector<std::pair<int64_t, bool>>; // [(value, isTainted), (value, isTainted), ...]
    
    std::vector<std::pair<Output, SimpleTaint>> BLOCKSCI_EXPORT getPoisonTainted(std::vector<Output> &outputs, BlockHeight maxBlockHeight, bool taintFee);
    std::vector<std::pair<Output, SimpleTaint>> BLOCKSCI_EXPORT getHaircutTainted(std::vector<Output> &outputs, BlockHeight maxBlockHeight, bool taintFee);
}}

#endif /* taint_hpp */
