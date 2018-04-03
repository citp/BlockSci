//
//  tx_identification.cpp
//  blocksci
//
//  Created by Harry Kalodner on 12/1/17.
//

#include "tx_identification.hpp"
#include <blocksci/chain/transaction.hpp>
#include <blocksci/util/hash.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <unordered_set>
#include <unordered_map>

namespace blocksci {
namespace heuristics {
    
    bool isCoinjoin(const Transaction &tx) {
        if (tx.inputCount() < 2 || tx.outputCount() < 3) {
            return false;
        }
        
        uint16_t participantCount = (tx.outputCount() + 1) / 2;
        if (participantCount > tx.inputCount()) {
            return false;
        }
        
        std::unordered_set<Address> inputAddresses;
        for (auto input : tx.inputs()) {
            inputAddresses.insert(input.getAddress());
        }
        
        if (participantCount > inputAddresses.size()) {
            return false;
        }
        
        std::unordered_map<uint64_t, uint16_t> outputValues;
        for (auto output : tx.outputs()) {
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
        
        explicit OutputBucket(uint64_t goal) : currentValue(0), goalValue(goal) {}
        
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
        for (auto input : tx.inputs()) {
            inputValues[input.getAddress()] += input.getValue();
        }
        
        if (participantCount > inputValues.size()) {
            return CoinJoinResult::False;
        }
        
        std::unordered_map<uint64_t, std::unordered_set<Address>> outputValues;
        for (auto output : tx.outputs()) {
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
        for (auto output : tx.outputs()) {
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
        for (auto output : tx.outputs()) {
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
        for (auto input : tx.inputs()) {
            inputValues[input.getAddress()] += input.getValue();
        }
        
        if (inputValues.size() == 1) {
            return CoinJoinResult::False;
        }
        
        std::vector<Output> unknownOutputs;
        for (auto output : tx.outputs()) {
            if (inputValues.find(output.getAddress()) == inputValues.end()) {
                unknownOutputs.push_back(output);
            }
        }
        
        if (unknownOutputs.size() <= 1) {
            return CoinJoinResult::False;
        }
        
        outputValues.clear();
        for (auto &output : unknownOutputs) {
            outputValues[output.getValue()]++;
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
    
    bool isDeanonTx(const Transaction &tx) {
        if (tx.isCoinbase()) {
            return false;
        }
        
        if (tx.outputCount() == 1) {
            return false;
        }
        
        std::unordered_set<AddressType::Enum> inputCounts;
        for (auto input : tx.inputs()) {
            inputCounts.insert(input.getAddress().type);
        }
        
        if (inputCounts.size() != 1) {
            return false;
        }
        
        AddressType::Enum inputType = *inputCounts.begin();
        
        bool seenType = false;
        for (auto output : tx.outputs()) {
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
        ranges::optional<Address> getInsidePointer(const ranges::optional<Address> &address, const DataAccess &access);
        ranges::optional<Address> getInsidePointer(const Address &pointer, const DataAccess &access) {
            if (pointer.type == AddressType::Enum::SCRIPTHASH) {
                script::ScriptHash scriptHashAddress(pointer.scriptNum, access);
                return getInsidePointer(scriptHashAddress.getWrappedAddress(), access);
            } else {
                return pointer;
            }
        }
        
        ranges::optional<Address> getInsidePointer(const ranges::optional<Address> &address, const DataAccess &access) {
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
        
        DetailedType(const Address &pointer, const DataAccess &access) : mainType(pointer.type), hasSubtype(false), subType(AddressType::Enum::NONSTANDARD), i(0), j(0) {
            auto insidePointer = getInsidePointer(pointer, access);
            if (insidePointer) {
                subType = insidePointer->type;
                hasSubtype = true;
                if (subType == AddressType::Enum::MULTISIG) {
                    script::Multisig multisigAddress(insidePointer->scriptNum, access);
                    i = multisigAddress.getRequired();
                    j = static_cast<int>(multisigAddress.getAddresses().size());
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
    
    bool isChangeOverTx(const Transaction &tx) {
        if (tx.isCoinbase()) {
            return false;
        }
        
        std::unordered_set<DetailedType, DetailedTypeHasher> outputTypes;
        for (auto output : tx.outputs()) {
            outputTypes.insert(DetailedType{output.getAddress(), tx.getAccess()});
        }
        
        if (outputTypes.size() != 1) {
            return false;
        }
        
        std::unordered_set<DetailedType, DetailedTypeHasher> inputTypes;
        for (auto input : tx.inputs()) {
            inputTypes.insert(DetailedType{input.getAddress(), tx.getAccess()});
        }
        
        if (inputTypes.size() != 1) {
            return false;
        }
        
        return *outputTypes.begin() != *inputTypes.begin();
    }
    
    bool containsKeysetChange(const Transaction &tx) {
        if (tx.isCoinbase()) {
            return false;
        }
        
        std::unordered_set<Address> multisigOutputs;
        for (auto output : tx.outputs()) {
            auto pointer = getInsidePointer(output.getAddress(), tx.getAccess());
            if (pointer && pointer->type == AddressType::Enum::MULTISIG) {
                multisigOutputs.insert(*pointer);
            }
        }
        
        if (multisigOutputs.size() == 0) {
            return false;
        }
        
        std::unordered_set<Address> multisigInputs;
        for (auto input : tx.inputs()) {
            auto pointer = getInsidePointer(input.getAddress(), tx.getAccess());
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
            pointer.getScript().visitPointers(visitFunc);
        }
        
        for (auto &pointer : multisigInputs) {
            bool foundMatch = false;
            std::function<void(const blocksci::Address &)> visitFunc = [&](const blocksci::Address &add) {
                if (containedOutputs.find(add) != containedOutputs.end()) {
                    foundMatch = true;
                    return;
                }
            };
            pointer.getScript().visitPointers(visitFunc);
            if (foundMatch) {
                return true;
            }
        }
        
        return false;
    }
}}
