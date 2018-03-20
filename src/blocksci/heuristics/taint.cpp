//
//  taint.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/20/18.
//

#include "taint.hpp"
#include <blocksci/chain/algorithms.hpp>

namespace blocksci {
    std::vector<TaintedOutput> getHaircutTainted(const TaintedOutput &taintedOutput) {
        std::vector<TaintedOutput> outputs;
        auto spendingTx = taintedOutput.output.getSpendingTx();
        if (spendingTx) {
            auto totalOut = static_cast<double>(totalOutputValue(*spendingTx));
            for (auto spendingOut : spendingTx->outputs()) {
                auto percentage = static_cast<double>(spendingOut.getValue()) / totalOut;
                auto value = static_cast<uint64_t>(percentage * static_cast<double>(taintedOutput.taintedValue));
                auto subTainted = getHaircutTainted({spendingOut, value});
                outputs.insert(outputs.end(), subTainted.begin(), subTainted.end());
            }
        } else {
            outputs.push_back(taintedOutput);
        }
        return outputs;
    }
}
