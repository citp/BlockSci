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

namespace blocksci {
    struct TaintedOutput {
        Output output;
        uint64_t taintedValue;
        
        TaintedOutput(const Output &output_, uint64_t value) : output(output_), taintedValue(value) {}
        TaintedOutput(const Output &output_) : TaintedOutput(output_, output_.getValue()) {}
    };
    
    std::vector<TaintedOutput> getHaircutTainted(const TaintedOutput &taintedOutput);
}

#endif /* taint_hpp */
