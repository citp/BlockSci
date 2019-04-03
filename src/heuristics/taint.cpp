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
    
    bool hasTaint(const std::vector<SimpleTaint> &t) {
        for(const auto &taint : t) {
            if(taint.first > 0) {
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
        int64_t totalVal;
        if(tx.isCoinbase()) {
            // coinbase transactions can destroy value by not claiming all input value + fees
           totalVal = totalOutputValue(tx);
        } else {
            totalVal = totalInputValue(tx);
        }
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
    
    struct InoutInfo {
        uint32_t txNum;
        OutputPointer pointer;
        
        bool operator<(const InoutInfo& other) const {
            return std::tie(txNum, pointer) < std::tie(other.txNum, other.pointer);
        }
        
        bool operator==(const InoutInfo& other) const {
            return txNum == other.txNum && pointer == other.pointer;
        }
    };
}}

namespace std {
    template<> struct BLOCKSCI_EXPORT hash<blocksci::heuristics::InoutInfo> {
        size_t operator()(const blocksci::heuristics::InoutInfo &info) const {
            std::size_t seed = 76547;
            blocksci::hash_combine(seed, info.txNum);
            blocksci::hash_combine(seed, info.pointer);
            return seed;
        }
    };
} // namespace std

namespace blocksci { namespace heuristics {
    template <typename Taint>
    using TaintMap = std::map<InoutInfo, Taint>;
    
    // Process an output and its associated taint value
    // If output has been spent, add the corresponding input to the list of tainted inputs
    // If output is unspent, add it to the list of tainted outputs
    template <typename Taint>
    void processOutput(TaintMap<Taint> &taintedInputs, std::unordered_map<OutputPointer, Taint> &taintedOutputs, const Output &spendingOut, Taint &newTaintedValue) {
        // ignore untainted or unspendable (OP_RETURN) outputs
        if (hasTaint(newTaintedValue) && spendingOut.getAddress().isSpendable()) {
            auto spendingTx = spendingOut.getSpendingTxIndex();
            if (spendingTx) {
                InoutInfo info{*spendingTx, spendingOut.pointer};
                auto it = taintedInputs.find(info);
                if (it == taintedInputs.end()) { // check if output was passed in originally as fully tainted
                    taintedInputs.insert(std::make_pair(info, std::move(newTaintedValue)));
                }
            } else {
                auto it = taintedOutputs.find(spendingOut.pointer);
                if (it == taintedOutputs.end()) { // check if output was passed in originally as fully tainted
                    taintedOutputs.insert(std::make_pair(spendingOut.pointer, std::move(newTaintedValue)));
                }
            }
        }
    }
    
    // Pass all outputs of a transaction and the corresponding taint to processOutput()
    template <typename Taint>
    void processTx(TaintMap<Taint> &taintedInputs, std::unordered_map<OutputPointer, Taint> &taintedOutputs, const Transaction &tx, std::vector<Taint> &outputTaint) {
        assert(outputTaint.size() == tx.outputCount());
        for (uint16_t i = 0; i < tx.outputCount(); i++) {
            processOutput(taintedInputs, taintedOutputs, tx.outputs()[i], outputTaint[i]);
        }
    }
    
    void clearTaint(SimpleTaint &taint) {
        taint.first = 0;
        taint.second = 0;
    }
    
    void clearTaint(ComplexTaint &taint) {
        taint.clear();
    }
    
    // Return the expected reward for a block
    int64_t getSubsidy(Block &block) {
        auto chainName = block.getAccess().config.chainConfig.coinName;
        int64_t subsidy;
        if(chainName == "bitcoin_regtest"){
            subsidy = 50 * COIN;
            if(block.height() >= 150){
                subsidy = 25 * COIN;
            }
        } else if(chainName == "bitcoin") {
            subsidy = 50 * COIN;
            int halvings = block.height() / 210000;
            // Force block reward to zero when right shift is undefined.
            if (halvings >= 64) {
                subsidy = 0;
            } else {
                subsidy >>= halvings;
            }
        } else {
            throw "Chain unsupported. You'll need to add chain-specific subsidy code to taint.cpp.";
        }
        
        return subsidy;
    }
    
    // Propagate taint
    template <typename Func, typename Taint>
    std::vector<std::pair<Output, Taint>> getTaintedImpl(Func func, std::vector<std::pair<Output, Taint>> &taintedOutputsRaw, BlockHeight maxBlockHeight, bool taintFee) {
        assert(taintedOutputsRaw.size() > 0);
        
        auto &access = taintedOutputsRaw[0].first.getAccess();
        
        TaintMap<Taint> taintedInputs;
        
        std::unordered_map<OutputPointer, Taint> taintedOutputs;
        
        if (maxBlockHeight == -1) {
            maxBlockHeight = access.getChain().blockCount();
        } else {
            // Range should include block at maxBlockHeight
            maxBlockHeight += 1;
            // Range shouldn't be larger than chain size
            maxBlockHeight = std::min(maxBlockHeight, access.getChain().blockCount());
        }
        
        BlockHeight minHeight = std::numeric_limits<BlockHeight>::max();
        for(std::pair<Output, Taint> &taintedOutput : taintedOutputsRaw){
            // Add outputs to map of tainted outputs
            processOutput(taintedInputs, taintedOutputs, taintedOutput.first, taintedOutput.second);
            
            minHeight = std::min(minHeight, taintedOutput.first.getBlockHeight());
        }
        
        BlockRange blocks{{minHeight, maxBlockHeight}, &access};
        std::vector<Taint> txOutputTaint;
        std::vector<Taint> txInputTaint;
        Taint coinbaseTaint;
        clearTaint(coinbaseTaint);
        for (auto block : blocks) {
            if (taintedInputs.begin()->first.txNum >= block.endTxIndex()) {
                continue;
            }
            std::vector<Taint> coinbaseTaintList;
            coinbaseTaintList.reserve(block.size());
            for (auto tx : block[{1, block.size()}]) {
                txOutputTaint.clear();
                txOutputTaint.reserve(tx.outputCount());
                clearTaint(coinbaseTaint);
                txInputTaint.clear();
                txInputTaint.reserve(tx.inputCount());
                
                // Transactions are processed in chronological order
                // If any input has taint, it must be at the beginning of the ordered taint map
                if (taintedInputs.begin()->first.txNum == tx.txNum) {
                    // Find tainted outputs spent in this transaction
                    for (auto input : tx.inputs()) {
                        InoutInfo info{tx.txNum, input.getSpentOutputPointer()};
                        auto it = taintedInputs.find(info);
                        if (it != taintedInputs.end()) {
                            txInputTaint.emplace_back(std::move(it->second));
                            taintedInputs.erase(it);
                        } else {
                            txInputTaint.emplace_back(UntaintedInputCreator<Taint>{}(input.getValue()));
                        }
                    }
                    
                    // Compute new taint of outputs
                    func(tx, txInputTaint, txOutputTaint, coinbaseTaint);
                    // Add new taint to taintedInputs/taintedOutputs
                    processTx(taintedInputs, taintedOutputs, tx, txOutputTaint);
                } else {
                    // No tainted inputs, thus fee is untainted
                    coinbaseTaint = UntaintedInputCreator<Taint>{}(tx.fee());
                }
                
                coinbaseTaintList.emplace_back(coinbaseTaint);
            }
            // If taintFee is false, all taint going into the coinbase transaction is discarded
            if (taintFee) {
                txOutputTaint.clear();
                txOutputTaint.reserve(block[0].outputCount());
                clearTaint(coinbaseTaint);
                
                int64_t subsidy = getSubsidy(block);
                coinbaseTaintList.insert(coinbaseTaintList.begin(), UntaintedInputCreator<Taint>{}(subsidy));
                func(block[0], coinbaseTaintList, txOutputTaint, coinbaseTaint);
                processTx(taintedInputs, taintedOutputs, block[0], txOutputTaint);
            }
        }
        
        std::vector<std::pair<Output, Taint>> ret;
        ret.reserve(taintedOutputs.size() + taintedInputs.size());
        
        // Tainted unspent outputs
        for (auto &item : taintedOutputs) {
            ret.emplace_back(Output{item.first, access}, item.second);
        }
        // Tainted spent outputs, but unspent at maxBlockHeight
        for (auto &item : taintedInputs) {
            ret.emplace_back(Output{item.first.pointer, access}, item.second);
        }
        return ret;
    }
    
    /**
     Implements poison tainting.
     Poison taint completely taints all outputs of a transaction.
     */
    std::vector<std::pair<Output, SimpleTaint>> getPoisonTainted(std::vector<Output> &outputs, BlockHeight maxBlockHeight, bool taintFee) {
        // Poison taint function
        auto poisonTaint = [](const Transaction &tx, const std::vector<SimpleTaint> &taintedInputs, std::vector<SimpleTaint> &outs, SimpleTaint &coinbaseTaint) {
            if(hasTaint(taintedInputs)) {
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

        // Outputs passed in are assumed to be fully tainted
        std::vector<std::pair<Output, SimpleTaint>> taint;
        taint.reserve(outputs.size());
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
        // Haircut taint function
        auto haircutTaint = [](const Transaction &tx, const std::vector<SimpleTaint> &taintedInputs, std::vector<SimpleTaint> &outs, SimpleTaint &coinbaseTaint) {
            int64_t totalTaintedVal = totalTaintedValue(tx, taintedInputs);
            auto totalIn = static_cast<double>(totalOutputValue(tx) + tx.fee());
            int64_t taintedValue = 0;
            for (auto spendingOut : tx.outputs()) {
                auto percentage = static_cast<double>(spendingOut.getValue()) / totalIn;
                auto newTaintedValue = std::min(static_cast<int64_t>(percentage * static_cast<double>(totalTaintedVal)), spendingOut.getValue());
                // make sure we don't taint more than what's left
                newTaintedValue = std::min(newTaintedValue, totalTaintedVal - taintedValue);
                taintedValue += newTaintedValue;
                outs.emplace_back(newTaintedValue, spendingOut.getValue() - newTaintedValue);
            }
            
            auto totalTxFee = tx.fee();
            auto feeTaint = std::min(totalTaintedVal - taintedValue, tx.fee());
            coinbaseTaint.first = feeTaint;
            coinbaseTaint.second = totalTxFee - feeTaint;
        };

        // Outputs passed in are assumed to be fully tainted
        std::vector<std::pair<Output, SimpleTaint>> taint;
        taint.reserve(outputs.size());
        for(const auto &output : outputs) {
            taint.emplace_back(output, SimpleTaint{output.getValue(), 0});
        }

        return getTaintedImpl(haircutTaint, taint, maxBlockHeight, taintFee);
    }
}}
