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
#include "chain_access.hpp"
#include "input_pointer.hpp"
#include "output_pointer.hpp"
#include "address/address.hpp"
#include "scripts/scriptsfwd.hpp"
#include "scripts/scripthash_script.hpp"
#include "scripts/multisig_script.hpp"

#include <boost/range/adaptors.hpp>
#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <sstream>

namespace blocksci {
    
    RawTransaction::RawTransaction(uint32_t sizeBytes_, uint32_t locktime_, uint16_t inputCount_, uint16_t outputCount_) : sizeBytes(sizeBytes_), locktime(locktime_), inputCount(inputCount_), outputCount(outputCount_) {}
    
    Transaction::Transaction(const RawTransaction *data_, uint32_t txNum_, uint32_t blockHeight_) : data(data_), txNum(txNum_), blockHeight(blockHeight_) {}
    
    const Transaction &Transaction::create(const ChainAccess &access, uint32_t index) {
        return *reinterpret_cast<const Transaction *>(access.getTxPos(index));
    }
    
    uint256 Transaction::getHash(const ChainAccess &access) const {
        auto &txHashesFile = access.getTxHashesFile();
        return *txHashesFile.getData(txNum);
    }
    
    const Block &Transaction::block(const ChainAccess &access) const {
        return access.getBlock(blockHeight);
    }
    
    std::string Transaction::getString() const {
        std::stringstream ss;
        ss << "Tx(len(txins)=" << inputCount() <<", len(txouts)=" << outputCount() <<", size_bytes=" << sizeBytes() << ", block_height=" << blockHeight <<", tx_index=" << txNum << ")";
        return ss.str();
    }
    
    boost::optional<Transaction> Transaction::txWithHash(const ChainAccess &access, uint256 hash) {
        auto &txHashesFile = access.getTxHashesFile();
        auto index = txHashesFile.find([&](const uint256 &hash2) { return hash == hash2; });
        if (index) {
            return txWithIndex(access, *index);
        }
        return boost::none;
    }
    
    std::vector<Transaction> getTransactionsFromHashes(const ChainAccess &access, const std::vector<std::string> &txHashes) {
        std::vector<Transaction> txes;
        std::unordered_set<uint256> hashes;
        for (auto &txHash : txHashes) {
            hashes.insert(uint256S(txHash));
        }
        auto &txHashesFile = access.getTxHashesFile();
        auto indexes = txHashesFile.findAll([&](const uint256 &hash) {
            return hashes.find(hash) != hashes.end();
        });
        
        txes.reserve(indexes.size());
        for (auto txNum : indexes) {
            txes.push_back(Transaction::create(access, txNum));
        }
        
        return txes;
    }
    
    Transaction Transaction::txWithIndex(const ChainAccess &access, uint32_t index) {
        return txWithIndex(access, index, access.getBlockHeight(index));
    }
    
    Transaction Transaction::txWithIndex(const ChainAccess &access, uint32_t index, uint32_t height) {
        return {access.getTx(index), index, height};
    }
    
    boost::optional<Transaction> Transaction::txWithHash(const ChainAccess &access, std::string hash) {
        return txWithHash(access, uint256S(hash));
    }
    
    Transaction::output_range Transaction::outputs() const {
        auto pos = reinterpret_cast<const char *>(data) + sizeof(RawTransaction);
        return boost::make_iterator_range_n(reinterpret_cast<const Output *>(pos), outputCount());
    }
    
    Transaction::input_range Transaction::inputs() const {
        auto pos = reinterpret_cast<const char *>(data) + sizeof(RawTransaction) + outputCount() * sizeof(Output);
        return boost::make_iterator_range_n(reinterpret_cast<const Input *>(pos), inputCount());
    }
    
    std::vector<OutputPointer> Transaction::getOutputPointers(const InputPointer &pointer, const ChainAccess &access) const {
        std::vector<OutputPointer> pointers;
        auto search = pointer.getOutput(access);
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
        auto search = pointer.getInput(access);
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
        
        int j = 0;
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
    
    const Output *getChangeOutput(const ScriptFirstSeenAccess &scripts, const Transaction &tx) {
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
                if (output.getValue() < smallestInput && output.getAddress().getFirstTransactionIndex(scripts) == tx.txNum) {
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
        Address getInsidePointer(const Address &pointer, const blocksci::ScriptAccess &access) {
            if (pointer.type == AddressType::Enum::SCRIPTHASH) {
                auto address = pointer.getScript(access);
                auto scriptHashAddress = dynamic_cast<script::ScriptHash *>(address.get());
                return getInsidePointer(scriptHashAddress->wrappedAddress, access);
            } else {
                return pointer;
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
            subType = insidePointer.type;
            hasSubtype = insidePointer.addressNum > 0;
            if (subType == AddressType::Enum::MULTISIG) {
                auto address = insidePointer.getScript(scripts);
                auto multisigAddress = dynamic_cast<script::Multisig *>(address.get());
                i = multisigAddress->required;
                j = multisigAddress->addresses.size();
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
            boost::hash_combine(seed, b.mainType);
            if (b.hasSubtype) {
                boost::hash_combine(seed, b.subType);
            }
            if (b.subType == AddressType::Enum::MULTISIG) {
                boost::hash_combine(seed, b.i);
                boost::hash_combine(seed, b.j);
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
            if (pointer.type == AddressType::Enum::MULTISIG) {
                multisigOutputs.insert(pointer);
            }
        }
        
        if (multisigOutputs.size() == 0) {
            return false;
        }
        
        std::unordered_set<Address> multisigInputs;
        for (auto &input : tx.inputs()) {
            auto pointer = getInsidePointer(input.getAddress(), access);
            if (pointer.type == AddressType::Enum::MULTISIG) {
                if (multisigOutputs.find(pointer) == multisigOutputs.end()) {
                    multisigInputs.insert(pointer);
                }
            }
        }
        
        if (multisigInputs.size() == 0) {
            return false;
        }
        
        std::unordered_set<Address> containedOutputs;
        for (auto &pointer : multisigOutputs) {
            auto address = pointer.getScript(access);
            auto multisigAddress = dynamic_cast<script::Multisig *>(address.get());
            for (auto &add : multisigAddress->nestedAddresses()) {
                containedOutputs.insert(add);
            }
        }
        
        for (auto &pointer : multisigInputs) {
            auto address = pointer.getScript(access);
            auto multisigAddress = dynamic_cast<script::Multisig *>(address.get());
            for (auto &add : multisigAddress->nestedAddresses()) {
                if (containedOutputs.find(add) != containedOutputs.end()) {
                    return true;
                }
            }
        }
        
        return false;
    }
}
