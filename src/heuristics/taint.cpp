//
//  taint.cpp
//  blocksci
//
//  Created by Harry Kalodner on 3/20/18.
//

#include <blocksci/heuristics/taint.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/block_range.hpp>

#include <internal/data_access.hpp>
#include <internal/chain_access.hpp>

#include <numeric>
#include <map>
#include <unordered_map>

#include <iostream>

namespace blocksci { namespace heuristics {
    
    constexpr int64_t COIN = 1e8;
    
    bool hasTaint(const SimpleTaint &val) {
        return val.first > 0;
    }
    
    bool hasTaint(const ComplexTaint &val) {
        for (const auto &it : val) {
            if (it.second && it.first > 0) {
                return true;
            }
        }
        return false;
    }
    
    int64_t totalValue(const SimpleTaint &taint) {
        return taint.first + taint.second;
    }
    
    int64_t totalValue(const ComplexTaint &taint) {
        int64_t total = 0;
        for (const auto &info : taint) {
            total += info.first;
        }
        return total;
    }

    int64_t totalTaintedValue(const Transaction &tx, const std::vector<SimpleTaint> &taintedInputs) {
        int64_t taintedValue = 0;
        int64_t totalVal = totalOutputValue(tx) + tx.fee();
        for(auto taint : taintedInputs) {
            int64_t newTaintedValue = std::min(taint.first, totalVal);
            taintedValue += newTaintedValue;
            totalVal -= totalValue(taint);
            if(totalVal <= 0) {
                break;
            }
        }
        return taintedValue;
    }
    
    template<typename Taint>
    struct UntaintedInputCreator {
        Taint operator()(int64_t value);
    };
    
    template<>
    SimpleTaint UntaintedInputCreator<SimpleTaint>::operator()(int64_t value) {
        return {0, value};
    }
    
    template<>
    ComplexTaint UntaintedInputCreator<ComplexTaint>::operator()(int64_t value) {
        ComplexTaint taint;
        taint.reserve(1);
        taint.emplace_back(value, false);
        return taint;
    }
    
    template <typename Taint>
    void processOutput(std::unordered_map<OutputPointer, Taint> &taintedOutputs, const Output &spendingOut, Taint &newTaintedValue) {
        if (hasTaint(newTaintedValue) && spendingOut.getAddress().isSpendable()) {
            auto it = taintedOutputs.find(spendingOut.pointer);
            if (it == taintedOutputs.end()) { // check if output was passed in originally as fully tainted
                taintedOutputs.insert(std::make_pair(spendingOut.pointer, std::move(newTaintedValue)));
            }
        }
    }
    
    template <typename Taint>
    void processTx(std::unordered_map<OutputPointer, Taint> &taintedOutputs, const Transaction &tx, std::vector<Taint> &outputTaint) {
        assert(outputTaint.size() == tx.outputCount());
        for (uint16_t i = 0; i < tx.outputCount(); i++) {
            processOutput(taintedOutputs, tx.outputs()[i], outputTaint[i]);
        }
    }
    
    template <typename Taint>
    std::vector<std::pair<Output, Taint>> convertFinalOutputs(const std::unordered_map<OutputPointer, Taint> &taintedOutputs, DataAccess &access) {
        std::vector<std::pair<Output, Taint>> ret;
        ret.reserve(taintedOutputs.size());
        for (auto &item : taintedOutputs) {
            ret.emplace_back(Output{item.first, access}, item.second);
        }
        return ret;
    }
    
    void clearTaint(SimpleTaint &taint) {
        taint.first = 0;
        taint.second = 0;
    }
    
    void clearTaint(ComplexTaint &taint) {
        taint.clear();
    }
    
    int64_t getSubsidy(Block &block) {
        auto chainName = block.getAccess().config.chainConfig.coinName;
        int64_t subsidy;
        if(chainName == "bitcoin_regtest"){
            subsidy = 50 * COIN;
            if(block.height() >= 150){
                subsidy = 25 * COIN;
            }
        } else {
            subsidy = 50 * COIN;
            int halvings = block.height() / 210000;
            // Force block reward to zero when right shift is undefined.
            if (halvings >= 64) {
                subsidy = 0;
            } else {
                subsidy >>= halvings;
            }
        }
        
        return subsidy;
    }
    
    template <typename Func, typename Taint>
    std::vector<std::pair<Output, Taint>> getTaintedImpl(Func func, std::vector<std::pair<Output, Taint>> &taintedOutputsRaw, BlockHeight maxBlockHeight, bool taintFee) {
        assert(taintedOutputsRaw.size() > 0);
        
        auto &access = taintedOutputsRaw[0].first.getAccess();
        
        std::unordered_map<OutputPointer, Taint> taintedOutputs;
        
        if (maxBlockHeight == -1) {
            maxBlockHeight = access.getChain().blockCount();
        } else {
            maxBlockHeight += 1; // range should include block at maxBlockHeight
        }
        
        BlockHeight minHeight = std::numeric_limits<BlockHeight>::max();
        for(std::pair<Output, Taint> &taintedOutput : taintedOutputsRaw){
            processOutput(taintedOutputs, taintedOutput.first, taintedOutput.second);
            
            minHeight = std::min(minHeight, taintedOutput.first.getBlockHeight());
        }
        
        BlockRange blocks{{minHeight, maxBlockHeight}, &access};
        std::vector<Taint> txOutputTaint;
        std::vector<Taint> txInputTaint;
        Taint coinbaseTaint;
        clearTaint(coinbaseTaint);
        for (auto block : blocks) {
            std::vector<Taint> coinbaseTaintList;
            coinbaseTaintList.reserve(block.size());
            for (auto tx : block[{1, block.size()}]) {
                txOutputTaint.clear();
                txOutputTaint.reserve(tx.outputCount());
                clearTaint(coinbaseTaint);
                txInputTaint.clear();
                txInputTaint.reserve(tx.inputCount());
                bool hasTaintedInputs = false;
                for (auto input : tx.inputs()) {
                    auto it = taintedOutputs.find(input.getSpentOutputPointer());
                    if (it != taintedOutputs.end()) {
                        hasTaintedInputs = true;
                        txInputTaint.emplace_back(std::move(it->second));
                        taintedOutputs.erase(it);
                    } else {
                        txInputTaint.emplace_back(UntaintedInputCreator<Taint>{}(input.getValue()));
                    }
                }
                if(hasTaintedInputs) {
                    func(tx, txInputTaint, txOutputTaint, coinbaseTaint);
                    processTx(taintedOutputs, tx, txOutputTaint);    
                } else {
                    coinbaseTaint = UntaintedInputCreator<Taint>{}(tx.fee());
                }
                coinbaseTaintList.emplace_back(coinbaseTaint);
            }
            if (taintFee) {
                txOutputTaint.clear();
                txOutputTaint.reserve(block[0].outputCount());
                clearTaint(coinbaseTaint);
                
                int64_t subsidy = getSubsidy(block);
                coinbaseTaintList.insert(coinbaseTaintList.begin(), UntaintedInputCreator<Taint>{}(subsidy));
                func(block[0], coinbaseTaintList, txOutputTaint, coinbaseTaint);
                processTx(taintedOutputs, block[0], txOutputTaint);
            }
        }
        return convertFinalOutputs(taintedOutputs, access);
    }
    
    /**
     Implements poison tainting.
     Poison taint completely taints all outputs of a transaction.
     */
    std::vector<std::pair<Output, SimpleTaint>> getPoisonTainted(std::vector<Output> &outputs, BlockHeight maxBlockHeight, bool taintFee) {
        auto poisonTaint = [](const Transaction &tx, const std::vector<SimpleTaint> &taintedInputs, std::vector<SimpleTaint> &outs, SimpleTaint &coinbaseTaint) {
            if(totalTaintedValue(tx, taintedInputs) > 0) {
                for (auto spendingOut : tx.outputs()) {
                    outs.emplace_back(spendingOut.getValue(), 0);
                }
                coinbaseTaint.first = tx.fee();    
            } else {
                for (auto spendingOut : tx.outputs()) {
                    outs.emplace_back(0, spendingOut.getValue());
                }
                coinbaseTaint.second = tx.fee();
            }
        };

        std::vector<std::pair<Output, SimpleTaint>> taint;
        for(const auto &output : outputs) {
            taint.emplace_back(output, SimpleTaint{output.getValue(), 0});
        }

        return getTaintedImpl(poisonTaint, taint, maxBlockHeight, taintFee);
    }
    
    /**
     Implements haircut tainting.
     Haircut taint applies all input taint uniformly distributed to the outputs.
     */
    std::vector<std::pair<Output, SimpleTaint>> getHaircutTainted(std::vector<Output> &outputs, BlockHeight maxBlockHeight, bool taintFee) {
        auto haircutTaint = [](const Transaction &tx, const std::vector<SimpleTaint> &taintedInputs, std::vector<SimpleTaint> &outs, SimpleTaint &coinbaseTaint) {
            int64_t totalTaintedVal = totalTaintedValue(tx, taintedInputs);
            auto totalIn = static_cast<double>(totalOutputValue(tx) + tx.fee());
            int64_t taintedValue = 0;
            for (auto spendingOut : tx.outputs()) {
                auto percentage = static_cast<double>(spendingOut.getValue()) / totalIn;
                auto newTaintedValue = std::min(static_cast<int64_t>(percentage * static_cast<double>(totalTaintedVal)), spendingOut.getValue());
                taintedValue += newTaintedValue;
                outs.emplace_back(newTaintedValue, spendingOut.getValue() - newTaintedValue);
            }
            
            auto totalTxFee = tx.fee();
            auto feeTaint = std::min(totalTaintedVal - taintedValue, tx.fee());
            coinbaseTaint.first = feeTaint;
            coinbaseTaint.second = totalTxFee - feeTaint;
        };

        std::vector<std::pair<Output, SimpleTaint>> taint;
        for(const auto &output : outputs) {
            taint.emplace_back(output, SimpleTaint{output.getValue(), 0});
        }

        return getTaintedImpl(haircutTaint, taint, maxBlockHeight, taintFee);
    }
}}
