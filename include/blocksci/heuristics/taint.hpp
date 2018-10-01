//
//  taint.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/20/18.
//

#ifndef taint_hpp
#define taint_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/chain_fwd.hpp>

#include <cstdint>
#include <vector>

namespace blocksci { namespace heuristics {
    std::vector<std::pair<Output, int64_t>> BLOCKSCI_EXPORT getPoisonTainted(const Output &output, int64_t taintedValue, int64_t maxBlockHeight);
    std::vector<std::pair<Output, int64_t>> BLOCKSCI_EXPORT getHaircutTainted(const Output &output, int64_t taintedValue, int64_t maxBlockHeight);
    std::vector<std::pair<Output, int64_t>> BLOCKSCI_EXPORT getFifoTainted(const Output &output, int64_t taintedValue, int64_t maxBlockHeight);
}}

#endif /* taint_hpp */
