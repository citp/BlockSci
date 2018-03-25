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
#include <unordered_map>

namespace blocksci { namespace heuristics {
    template <typename Func>
    std::vector<std::pair<Output, uint64_t>> getTaintedImpl(Func func, const Output &output, uint64_t taintedValue) {
        std::map<uint32_t, std::vector<std::pair<Inout, uint64_t>>> taintedTxesToCheck;
        std::vector<std::pair<Output, uint64_t>> taintedOutputs;
        auto processOutput = [&](const Output &spendingOut, uint64_t newTaintedValue) {
            if (newTaintedValue > 0) {
                if (spendingOut.isSpent()) {
                    auto &txData = taintedTxesToCheck[spendingOut.getSpendingTxIndex()];
                    auto newTaintedInput = Inout{spendingOut.pointer.txNum, spendingOut.getAddress(), spendingOut.getValue()};
                    txData.emplace_back(newTaintedInput, newTaintedValue);
                } else {
                    if (spendingOut.getAddress().isSpendable()) {
                        taintedOutputs.emplace_back(spendingOut, newTaintedValue);
                    }
                }
            }
        };
        processOutput(output, taintedValue);
        while (!taintedTxesToCheck.empty()) {
            auto taintedTxData = *taintedTxesToCheck.begin();
            taintedTxesToCheck.erase(taintedTxesToCheck.begin());
            auto tx = Transaction{taintedTxData.first, output.getAccess()};
            auto &taintedInputs = taintedTxData.second;
            for (auto &taintedOut : func(tx, taintedInputs)) {
                processOutput(taintedOut.first, taintedOut.second);
            }
        }
        return taintedOutputs;
    }
    
    std::vector<std::pair<Output, uint64_t>> getPoisonTainted(const Output &output, uint64_t taintedValue) {
        auto poisonTaint = [](const Transaction &tx, std::vector<std::pair<Inout, uint64_t>> &) {
            std::vector<std::pair<Output, uint64_t>> outs;
            for (auto spendingOut : tx.outputs()) {
                outs.emplace_back(spendingOut, spendingOut.getValue());
            }
            return outs;
        };
        return getTaintedImpl(poisonTaint, output, taintedValue);
    }
    
    std::vector<std::pair<Output, uint64_t>> getHaircutTainted(const Output &output, uint64_t taintedValue) {
        auto haircutTaint = [](const Transaction &tx, std::vector<std::pair<Inout, uint64_t>> &taintedInputs) {
            std::vector<std::pair<Output, uint64_t>> outs;
            uint64_t taintedValue = 0;
            for (auto &pair : taintedInputs) {
                taintedValue += pair.second;
            }
            auto totalIn = static_cast<double>(totalInputValue(tx));
            for (auto spendingOut : tx.outputs()) {
                auto percentage = static_cast<double>(spendingOut.getValue()) / totalIn;
                auto newTaintedValue = std::min(static_cast<uint64_t>(percentage * static_cast<double>(taintedValue)), spendingOut.getValue());
                outs.emplace_back(spendingOut, newTaintedValue);
            }
            return outs;
        };
        return getTaintedImpl(haircutTaint, output, taintedValue);
    }
    
    std::vector<std::pair<Output, uint64_t>> getFifoTainted(const Output &output, uint64_t taintedValue) {
        auto fifoTaint = [](const Transaction &tx, std::vector<std::pair<Inout, uint64_t>> &taintedInputs) {
            std::vector<std::pair<Output, uint64_t>> outs;
            uint16_t currentOutputNum = 0;
            uint64_t currentOutputTaintValue = 0;
            uint64_t currentOutputValueLeft = tx.outputs()[currentOutputNum].getValue();
            auto addVal = [&](auto &val, bool isTaint) -> bool {
                uint64_t valToAdd = std::min(val, currentOutputValueLeft);
                currentOutputValueLeft -= valToAdd;
                val -= valToAdd;
                if (isTaint) {
                    currentOutputTaintValue += valToAdd;
                }
                if (currentOutputValueLeft == 0) {
                    outs.emplace_back(tx.outputs()[currentOutputNum], currentOutputTaintValue);
                    currentOutputNum++;
                    if (currentOutputNum == tx.outputCount()) {
                        return false;
                    } else {
                        currentOutputTaintValue = 0;
                        currentOutputValueLeft = tx.outputs()[currentOutputNum].getValue();
                    }
                }
                return true;
            };
            std::unordered_map<Inout, uint64_t> taintedInputsMap;
            for (auto &pair : taintedInputs) {
                auto inserted = taintedInputsMap.insert(pair).second;
                if (!inserted) {
                    throw std::runtime_error{"Error: Cannot distinguish between inputs"};
                }
            }
            for (auto input : tx.inputs()) {
                auto it = taintedInputsMap.find(Inout(input.spentTxIndex(), input.getAddress(), input.getValue()));
                uint64_t taintedValue = 0;
                if (it != taintedInputsMap.end()) {
                    taintedValue = it->second;
                }
                uint64_t untaintedValue = input.getValue() - taintedValue;
                while (taintedValue > 0 ) {
                    if (!addVal(taintedValue, true)) {
                        return outs;
                    }
                }
                while (untaintedValue > 0) {
                    if (!addVal(taintedValue, false)) {
                        return outs;
                    }
                }
            }
            return outs;
        };
        return getTaintedImpl(fifoTaint, output, taintedValue);
    }
}}
