//
//  transaction.cpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#define BLOCKSCI_WITHOUT_SINGLETON

#include "transaction.hpp"
#include "output.hpp"
#include "input.hpp"
#include "block.hpp"
#include "chain_access.hpp"
#include "input_pointer.hpp"
#include "output_pointer.hpp"
#include "address/address.hpp"
#include "scripts/scriptsfwd.hpp"
#include "scripts/script_variant.hpp"
#include "hash_index.hpp"

#include "hash.hpp"

#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <sstream>

namespace blocksci {
    
    RawTransaction::RawTransaction(uint32_t sizeBytes_, uint32_t locktime_, uint16_t inputCount_, uint16_t outputCount_) : sizeBytes(sizeBytes_), locktime(locktime_), inputCount(inputCount_), outputCount(outputCount_), inOuts(static_cast<uint32_t>(inputCount_ + outputCount_)) {}
    
    Output &RawTransaction::getOutput(uint16_t outputNum) {
        return reinterpret_cast<Output &>(inOuts[inputCount + outputNum]);
    }
    
    Input &RawTransaction::getInput(uint16_t inputNum) {
        return reinterpret_cast<Input &>(inOuts[inputNum]);
    }
    
    const Output &RawTransaction::getOutput(uint16_t outputNum) const {
        return reinterpret_cast<const Output &>(inOuts[inputCount + outputNum]);
    }
    
    const Input &RawTransaction::getInput(uint16_t inputNum) const {
        return reinterpret_cast<const Input &>(inOuts[inputNum]);
    }
    
    
    Transaction::Transaction(const ChainAccess &access_, const RawTransaction *data_, uint32_t txNum_, uint32_t blockHeight_) : access(&access_), data(data_), txNum(txNum_), blockHeight(blockHeight_) {}
    
    Transaction::Transaction(const ChainAccess &access_, uint32_t index) : Transaction(access_, index, access_.getBlockHeight(index)) {}
    
    Transaction::Transaction(const ChainAccess &access_, uint32_t index, uint32_t height) : Transaction(access_, access_.getTx(index), index, height) {}
    
    uint256 Transaction::getHash(const ChainAccess &access) const {
        auto &txHashesFile = access.getTxHashesFile();
        return *txHashesFile.getData(txNum);
    }
    
    Block Transaction::block(const ChainAccess &access) const {
        return Block(blockHeight, access);
    }
    
    std::string Transaction::getString() const {
        std::stringstream ss;
        ss << "Tx(len(txins)=" << inputCount() <<", len(txouts)=" << outputCount() <<", size_bytes=" << sizeBytes() << ", block_height=" << blockHeight <<", tx_index=" << txNum << ")";
        return ss.str();
    }
    
    ranges::optional<Transaction> Transaction::txWithHash(uint256 hash, const HashIndex &index, const ChainAccess &chain) {
        auto txXndex = index.getTxIndex(hash);
        if (txXndex != 0) {
            return Transaction(chain, txXndex);
        } else {
            return ranges::nullopt;
        }
    }
    
    ranges::optional<Transaction> Transaction::txWithHash(std::string hash, const HashIndex &index, const ChainAccess &access) {
        return txWithHash(uint256S(hash), index, access);
    }
    
    Transaction::output_range Transaction::outputs() const {
        auto &firstOut = data->getOutput(0);
        return ranges::v3::make_iterator_range(&firstOut, &firstOut + outputCount());
    }
    
    Transaction::input_range Transaction::inputs() const {
        auto &firstIn = data->getInput(0);
        return ranges::v3::make_iterator_range(&firstIn, &firstIn + inputCount());
    }
    
    std::vector<OutputPointer> Transaction::getOutputPointers(const InputPointer &pointer, const ChainAccess &access) const {
        std::vector<OutputPointer> pointers;
        auto &input = pointer.getInput(access);
        auto search = Output{pointer.txNum, input.getAddress(), input.getValue()};
        uint16_t i = 0;
        for (auto &output : outputs()) {
            if (output == search) {
                pointers.emplace_back(txNum, i);
            }
            i++;
        }
        return pointers;
    }
    
    std::vector<InputPointer> Transaction::getInputPointers(const OutputPointer &pointer, const ChainAccess &access) const {
        std::vector<InputPointer> pointers;
        auto &output = pointer.getOutput(access);
        auto search = Input{pointer.txNum, output.getAddress(), output.getValue()};
        uint16_t i = 0;
        for (auto &input : inputs()) {
            if (input == search) {
                pointers.emplace_back(txNum, i);
            }
            i++;
        }
        return pointers;
    }
    
    bool isCoinbase(const Transaction &tx) {
        return tx.inputCount() == 0;
    }
    
    const Output *getOpReturn(const Transaction &tx) {
        for (auto &output : tx.outputs()) {
            if (output.getType() == AddressType::Enum::NULL_DATA) {
                return &output;
            }
        }
        return nullptr;
    }
    
    uint64_t totalOut(const Transaction &tx) {
        uint64_t total = 0;
        for (auto &output : tx.outputs()) {
            total += output.getValue();
        }
        return total;
    }
    
    uint64_t totalIn(const Transaction &tx) {
        uint64_t total = 0;
        for (auto &input : tx.inputs()) {
            total += input.getValue();
        }
        return total;
    }
    
    uint64_t fee(const Transaction &tx) {
        if (isCoinbase(tx)) {
            return 0;
        } else {
            return totalIn(tx) - totalOut(tx);
        }
    }
    
    bool hasFeeGreaterThan(const Transaction &tx, uint64_t txFee) {
        return fee(tx) > txFee;
    }
    
    double feePerByte(const Transaction &tx) {
        return static_cast<double>(fee(tx)) / static_cast<double>(tx.sizeBytes());
    }
    
    bool isCoinjoin(const Transaction &tx) {
        if (tx.inputCount() < 2 || tx.outputCount() < 3) {
            return false;
        }
        
        uint16_t participantCount = (tx.outputCount() + 1) / 2;
        if (participantCount > tx.inputCount()) {
            return false;
        }
        
        std::unordered_set<Address> inputAddresses;
        for (auto &input : tx.inputs()) {
            inputAddresses.insert(input.getAddress());
        }
        
        if (participantCount > inputAddresses.size()) {
            return false;
        }
        
        std::unordered_map<uint64_t, uint16_t> outputValues;
        for (auto &output : tx.outputs()) {
            outputValues[output.getValue()]++;
        }
        
        using pair_type = decltype(outputValues)::value_type;
        auto pr = std::max_element(std::begin(outputValues), std::end(outputValues),
                                   [] (const pair_type & p1, const pair_type & p2) {
                                       return p1.second < p2.second;
                                   }
                                   );
        
        if (pr->second != participantCount) {
            return false;
        }
        
        if (pr->first == 546 || pr->first == 2730) {
            return false;
        }
        
        return true;
    }
    
    struct OutputBucket {
        uint64_t currentValue;
        uint64_t goalValue;
        
        OutputBucket(uint64_t goal) : currentValue(0), goalValue(goal) {}
        
        bool isFull() const {
            return currentValue >= goalValue;
        }
        
        void addInput(uint64_t val) {
            currentValue += val;
        }
        
        void removeInput(uint64_t val) {
            currentValue -= val;
        }
        
        uint64_t remaining() const {
            if (goalValue > currentValue) {
                return goalValue - currentValue;
            } else {
                return 0;
            }
        }
        
        bool operator<(const OutputBucket &other) const {
            return remaining() < other.remaining();
        }
    };
    
    CoinJoinResult _getSumCount(std::vector<uint64_t> &values, std::vector<OutputBucket> buckets, uint64_t totalRemaining, uint64_t valueLeft, size_t maxDepth, size_t &depth);
    CoinJoinResult getSumCount(std::vector<uint64_t> &values, std::vector<uint64_t> bucketGoals, size_t maxDepth);
    
    
    CoinJoinResult _getSumCount(std::vector<uint64_t> &values, std::vector<OutputBucket> buckets, uint64_t totalRemaining, uint64_t valueLeft, size_t maxDepth, size_t &depth) {
        if (totalRemaining > valueLeft) {
            return CoinJoinResult::False;
        }
        
        buckets.erase(std::remove_if(buckets.begin(), buckets.end(), [&](auto &bucket) { return bucket.isFull(); }), buckets.end());
        if (buckets.size() == 0) {
            return CoinJoinResult::True;
        }
        
        if (values.size() == 0) {
            return CoinJoinResult::False;
        }
        
        depth++;
        
        if (maxDepth != 0 && depth > maxDepth) {
            return CoinJoinResult::Timeout;
        }
        
        std::sort(buckets.rbegin(), buckets.rend());
        
        uint64_t lastValue = values.back();
        values.pop_back();
        valueLeft -= lastValue;
        for (auto &bucket : buckets) {
            uint64_t remaining = totalRemaining - bucket.remaining();
            bucket.addInput(lastValue);
            remaining += bucket.remaining();
            auto res = _getSumCount(values, buckets, remaining, valueLeft, maxDepth, depth);
            if (res != CoinJoinResult::False) {
                return res;
            }
            bucket.removeInput(lastValue);
        }
        values.push_back(lastValue);
        return CoinJoinResult::False;
    }
    
    
    CoinJoinResult getSumCount(std::vector<uint64_t> &values, std::vector<uint64_t> bucketGoals, size_t maxDepth) {
        std::sort(values.begin(), values.end());
        std::sort(bucketGoals.rbegin(), bucketGoals.rend());
        
        uint64_t valueLeft = 0;
        for (auto value : values) {
            valueLeft += value;
        }
        
        uint64_t totalRemaining = 0;
        for (auto goal : bucketGoals) {
            totalRemaining += goal;
        }
        
        std::vector<OutputBucket> buckets;
        buckets.reserve(bucketGoals.size());
        for (auto goal : bucketGoals) {
            buckets.emplace_back(goal);
        }
        size_t depth = 0;
        return _getSumCount(values, buckets, totalRemaining, valueLeft, maxDepth, depth);
    }
    
    
    CoinJoinResult isCoinjoinExtra(const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth) {
        if (tx.inputCount() < 2 || tx.outputCount() < 3) {
            return CoinJoinResult::False;
        }
        
        uint16_t participantCount = (tx.outputCount() + 1) / 2;
        if (participantCount > tx.inputCount()) {
            return CoinJoinResult::False;
        }
        
        std::unordered_map<Address, uint64_t> inputValues;
        for (auto &input : tx.inputs()) {
            inputValues[input.getAddress()] += input.getValue();
        }
        
        if (participantCount > inputValues.size()) {
            return CoinJoinResult::False;
        }
        
        std::unordered_map<uint64_t, std::unordered_set<Address>> outputValues;
        for (auto &output : tx.outputs()) {
            outputValues[output.getValue()].insert(output.getAddress());
        }
        
        using pair_type = decltype(outputValues)::value_type;
        auto pr = std::max_element(std::begin(outputValues), std::end(outputValues),
                                   [] (const pair_type & p1, const pair_type & p2) {
                                       return p1.second.size() < p2.second.size();
                                   }
                                   );
        
        
        if (pr->second.size() != participantCount) {
            return CoinJoinResult::False;
        }
        
        if (pr->first == 546 || pr->first == 2730) {
            return CoinJoinResult::False;
        }
        
        
        std::vector<uint64_t> values;
        values.reserve(inputValues.size());
        for (auto &pair : inputValues) {
            values.push_back(pair.second);
        }
        
        uint64_t goalValue = pr->first;
        
        uint64_t maxPossibleFee = std::max(minBaseFee, static_cast<uint64_t>(goalValue * percentageFee));
        
        std::vector<uint64_t> bucketGoals;
        for (uint16_t i = 0; i < participantCount; i++) {
            bucketGoals.push_back(goalValue);
        }
        
        size_t j = 0;
        for (auto &output : tx.outputs()) {
            if (output.getValue() != goalValue) {
                bucketGoals[j] += output.getValue();
                j++;
            }
        }
        
        for (auto &goal : bucketGoals) {
            if (maxPossibleFee > goal) {
                goal = 0;
            } else {
                goal -= maxPossibleFee;
            }
        }
        
        return getSumCount(values, bucketGoals, maxDepth);
    }
    
    CoinJoinResult isPossibleCoinjoin(const Transaction &tx, uint64_t minBaseFee, double percentageFee, size_t maxDepth) {
        
        if (tx.outputCount() == 1 || tx.inputCount() == 1) {
            return CoinJoinResult::False;
        }
        
        std::unordered_map<uint64_t, uint16_t> outputValues;
        for (auto &output : tx.outputs()) {
            outputValues[output.getValue()]++;
        }
        
        using pair_type = decltype(outputValues)::value_type;
        auto pr = std::max_element(std::begin(outputValues), std::end(outputValues),
                                   [] (const pair_type & p1, const pair_type & p2) {
                                       return p1.second < p2.second;
                                   }
                                   );
        
        // There must be at least two outputs of equal value to create an anonymity set
        if (pr->second == 1) {
            return CoinJoinResult::False;
        }
        
        std::unordered_map<Address, uint64_t> inputValues;
        for (auto &input : tx.inputs()) {
            inputValues[input.getAddress()] += input.getValue();
        }
        
        if (inputValues.size() == 1) {
            return CoinJoinResult::False;
        }
        
        std::vector<const Output *> unknownOutputs;
        for (auto &output : tx.outputs()) {
            if (inputValues.find(output.getAddress()) == inputValues.end()) {
                unknownOutputs.push_back(&output);
            }
        }
        
        if (unknownOutputs.size() <= 1) {
            return CoinJoinResult::False;
        }
        
        outputValues.clear();
        for (auto &output : unknownOutputs) {
            outputValues[output->getValue()]++;
        }
        pr = std::max_element(std::begin(outputValues), std::end(outputValues),
                                   [] (const pair_type & p1, const pair_type & p2) {
                                       return p1.second < p2.second;
                                   }
                                   );
        // There must be at least two outputs of equal value to create an anonymity set
        if (pr->second == 1) {
            return CoinJoinResult::False;
        }
        
        std::vector<uint64_t> values;
        values.reserve(inputValues.size());
        for (auto &pair : inputValues) {
            values.push_back(pair.second);
        }
        
        uint64_t maxPossibleFee = std::max(minBaseFee, static_cast<uint64_t>(pr->first * percentageFee));
        uint64_t goalValue = 0;
        if (pr->first > goalValue) {
            goalValue = pr->first - maxPossibleFee;
        }
        
        std::vector<uint64_t> bucketGoals = {goalValue, goalValue};
        
        return getSumCount(values, bucketGoals, maxDepth);
    }
    
    const Output *getChangeOutput(const Transaction &tx, const ScriptAccess &scripts) {
        if (isCoinjoin(tx)) {
            return nullptr;
        }
        
        uint64_t smallestInput = std::numeric_limits<uint64_t>::max();
        for (auto &input : tx.inputs()) {
            smallestInput = std::min(smallestInput, input.getValue());
        }
        
        uint16_t spendableCount = 0;
        const Output *change = nullptr;
        for (const auto &output : tx.outputs()) {
            if (output.getAddress().isSpendable()) {
                spendableCount++;
                if (output.getValue() < smallestInput && output.getAddress().getScript(scripts).firstTxIndex() == tx.txNum) {
                    if (change) {
                        return nullptr;
                    }
                    change = &output;
                }
            }
        }
        if (change && spendableCount > 1) {
            return change;
        } else {
            return nullptr;
        }
    }
    
    bool isDeanonTx(const Transaction &tx) {
        if (isCoinbase(tx)) {
            return false;
        }
        
        if (tx.outputCount() == 1) {
            return false;
        }
        
        std::unordered_set<AddressType::Enum> inputCounts;
        for (auto &input : tx.inputs()) {
            inputCounts.insert(input.getAddress().type);
        }
        
        if (inputCounts.size() != 1) {
            return false;
        }
        
        AddressType::Enum inputType = *inputCounts.begin();
        
        bool seenType = false;
        for (auto &output : tx.outputs()) {
            if (output.getType() == inputType) {
                if (seenType) {
                    return false;
                }
                seenType = true;
            }
        }
        
        return seenType;
    }
    
    namespace {
        ranges::optional<Address> getInsidePointer(const ranges::optional<Address> &address, const blocksci::ScriptAccess &access);
        ranges::optional<Address> getInsidePointer(const Address &pointer, const blocksci::ScriptAccess &access) {
            if (pointer.type == AddressType::Enum::SCRIPTHASH) {
                script::ScriptHash scriptHashAddress(access, pointer.scriptNum);
                return getInsidePointer(scriptHashAddress.getWrappedAddress(), access);
            } else {
                return pointer;
            }
        }
        
        ranges::optional<Address> getInsidePointer(const ranges::optional<Address> &address, const blocksci::ScriptAccess &access) {
            if (address) {
                return getInsidePointer(*address, access);
            } else {
                return ranges::nullopt;
            }
        }
    }
    
    
    struct DetailedType {
        AddressType::Enum mainType;
        bool hasSubtype;
        AddressType::Enum subType;
        int i;
        int j;
        
        DetailedType(const Address &pointer, const ScriptAccess &scripts) : mainType(pointer.type), hasSubtype(false), subType(AddressType::Enum::NONSTANDARD), i(0), j(0) {
            auto insidePointer = getInsidePointer(pointer, scripts);
            if (insidePointer) {
                subType = insidePointer->type;
                hasSubtype = true;
                if (subType == AddressType::Enum::MULTISIG) {
                    script::Multisig multisigAddress(scripts, insidePointer->scriptNum);
                    i = multisigAddress.required;
                    j = static_cast<int>(multisigAddress.addresses.size());
                }
            }
        }
        
        bool operator==(const DetailedType &other) const {
            if (mainType != other.mainType || subType != other.subType) {
                return false;
            }
            if (mainType == AddressType::Enum::SCRIPTHASH && (!hasSubtype || !other.hasSubtype)) {
                return false;
            }
            
            if (subType == AddressType::Enum::MULTISIG && (i != other.i || j != other.j)) {
                return false;
            }
            
            return true;
        }
        
        bool operator!=(const DetailedType &other) const {
            return !operator==(other);
        }
    };
    
    struct DetailedTypeHasher {
        size_t operator()(const DetailedType& b) const {
            std::size_t seed = 123945432;
            hash_combine(seed, b.mainType);
            if (b.hasSubtype) {
                hash_combine(seed, b.subType);
            }
            if (b.subType == AddressType::Enum::MULTISIG) {
                hash_combine(seed, b.i);
                hash_combine(seed, b.j);
            }
            return seed;
        }
    };
    
    bool isChangeOverTx(const Transaction &tx, const ScriptAccess &scripts) {
        if (isCoinbase(tx)) {
            return false;
        }
        
        std::unordered_set<DetailedType, DetailedTypeHasher> outputTypes;
        for (auto &output : tx.outputs()) {
            outputTypes.insert(DetailedType{output.getAddress(), scripts});
        }
        
        if (outputTypes.size() != 1) {
            return false;
        }
        
        std::unordered_set<DetailedType, DetailedTypeHasher> inputTypes;
        for (auto &input : tx.inputs()) {
            inputTypes.insert(DetailedType{input.getAddress(), scripts});
        }
        
        if (inputTypes.size() != 1) {
            return false;
        }
        
        return *outputTypes.begin() != *inputTypes.begin();
    }
    
    bool containsKeysetChange(const Transaction &tx, const blocksci::ScriptAccess &access) {
        if (isCoinbase(tx)) {
            return false;
        }
        
        std::unordered_set<Address> multisigOutputs;
        for (auto &output : tx.outputs()) {
            auto pointer = getInsidePointer(output.getAddress(), access);
            if (pointer && pointer->type == AddressType::Enum::MULTISIG) {
                multisigOutputs.insert(*pointer);
            }
        }
        
        if (multisigOutputs.size() == 0) {
            return false;
        }
        
        std::unordered_set<Address> multisigInputs;
        for (auto &input : tx.inputs()) {
            auto pointer = getInsidePointer(input.getAddress(), access);
            if (pointer && pointer->type == AddressType::Enum::MULTISIG) {
                if (multisigOutputs.find(*pointer) == multisigOutputs.end()) {
                    multisigInputs.insert(*pointer);
                }
            }
        }
        
        if (multisigInputs.size() == 0) {
            return false;
        }
        
        std::unordered_set<Address> containedOutputs;
        for (auto &pointer : multisigOutputs) {
            std::function<void(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &add) {
                containedOutputs.insert(add);
            };
            pointer.getScript(access).visitPointers(visitFunc);
        }
        
        for (auto &pointer : multisigInputs) {
            bool foundMatch = false;
            std::function<void(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &add) {
                if (containedOutputs.find(add) != containedOutputs.end()) {
                    foundMatch = true;
                    return;
                }
            };
            pointer.getScript(access).visitPointers(visitFunc);
            if (foundMatch) {
                return true;
            }
        }
        
        return false;
    }
}
