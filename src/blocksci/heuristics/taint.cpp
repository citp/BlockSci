//
//  taint.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/20/18.
//

#include "taint.hpp"
#include <blocksci/address/address.hpp>
#include <blocksci/chain/algorithms.hpp>

#include <map>

namespace blocksci { namespace heuristics {
    std::vector<std::pair<Output, uint64_t>> getHaircutTainted(const Output &output, uint64_t taintedValue) {
        std::map<uint32_t, std::vector<std::pair<Inout, uint64_t>>> taintedTxesToCheck;
        std::vector<std::pair<Output, uint64_t>> taintedOutputs;
        auto processOutput = [&](const Output &spendingOut, uint64_t newTaintedValue) {
            if (spendingOut.isSpent()) {
                taintedTxesToCheck[spendingOut.getSpendingTxIndex()].emplace_back(Inout{spendingOut.pointer.txNum, spendingOut.getAddress(), spendingOut.getValue()}, newTaintedValue);
            } else {
                taintedOutputs.emplace_back(spendingOut, newTaintedValue);
            }
        };
        processOutput(output, taintedValue);
        while (!taintedTxesToCheck.empty()) {
            auto taintedTxData = *taintedTxesToCheck.begin();
            taintedTxesToCheck.erase(taintedTxesToCheck.begin());
            uint64_t taintedValue = 0;
            for (auto &pair : taintedTxData.second) {
                taintedValue += pair.second;
            }
            auto tx = Transaction(taintedTxData.first, output.getAccess());
            auto totalOut = static_cast<double>(totalOutputValue(tx));
            for (auto spendingOut : tx.outputs()) {
                auto percentage = static_cast<double>(spendingOut.getValue()) / totalOut;
                auto newTaintedValue = static_cast<uint64_t>(percentage * static_cast<double>(taintedValue));
                processOutput(spendingOut, newTaintedValue);
            }
        }
        return taintedOutputs;
    }
}}
