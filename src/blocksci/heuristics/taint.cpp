//
//  taint.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/20/18.
//

#include "taint.hpp"
#include <blocksci/chain/algorithms.hpp>

namespace blocksci { namespace heuristics {
    std::unordered_map<Output, uint64_t> getHaircutTainted(const Output &output, uint64_t taintedValue) {
        std::unordered_map<Output, uint64_t> outputs;
        std::vector<std::pair<Output, uint64_t>> outputsToCheck;
        outputsToCheck.emplace_back(output, taintedValue);
        while (!outputsToCheck.empty()) {
            auto tainted = outputsToCheck.back();
            outputsToCheck.pop_back();
            auto spendingTx = tainted.first.getSpendingTx();
            if (spendingTx) {
                auto totalOut = static_cast<double>(totalOutputValue(*spendingTx));
                for (auto spendingOut : spendingTx->outputs()) {
                    auto percentage = static_cast<double>(spendingOut.getValue()) / totalOut;
                    auto value = static_cast<uint64_t>(percentage * static_cast<double>(tainted.second));
                    outputsToCheck.emplace_back(spendingOut, value);
                }
            } else {
                outputs[tainted.first] += tainted.second;
            }
        }
        return outputs;
    }
}}
