//
//  inout_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include "inout_pointer.hpp"
#include "util/hash.hpp"
#include "chain_access.hpp"
#include "output.hpp"
#include "input.hpp"
#include "transaction.hpp"

#include <range/v3/view/transform.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/action/sort.hpp>

#include <unordered_set>
#include <sstream>

namespace blocksci {
    std::string InputPointer::toString() const {
        std::stringstream ss;
        ss << "InputPointer(tx_index=" << txNum << ", input_index=" << inoutNum << ")";
        return ss.str();
    }
    
    std::string OutputPointer::toString() const {
        std::stringstream ss;
        ss << "OutputPointer(tx_index_from=" << txNum << ", output_index_from=" << inoutNum << ")";
        return ss.str();
    }
    
    bool InputPointer::isValid(const ChainAccess &access) const {
        return inoutNum < access.getTx(txNum)->inputCount;
    }
    
    bool OutputPointer::isValid(const ChainAccess &access) const {
        return inoutNum < access.getTx(txNum)->outputCount;
    }
    
    uint64_t calculateBalance(const std::vector<OutputPointer> &pointers, BlockHeight height, const DataAccess &access) {
        uint64_t value = 0;
        if (height == -1) {
            for (auto &output : getOutputs(pointers, access)) {
                if (!output.isSpent()) {
                    value += output.getValue();
                }
            }
        } else {
            for (auto &output : getOutputs(pointers, access)) {
                if (output.blockHeight <= height && (!output.isSpent() || output.getSpendingTx()->blockHeight > height)) {
                    value += output.getValue();
                }
            }
        }
        return value;
    }
    
    std::vector<Output> getOutputs(const std::vector<OutputPointer> &pointers, const DataAccess &access) {
        return pointers
        | ranges::view::transform([&access](const OutputPointer &pointer) { return Output(pointer, access); })
        | ranges::to_vector;
    }
    
    std::vector<Input> getInputs(const std::vector<OutputPointer> &pointers, const DataAccess &access) {
        std::unordered_set<InputPointer> allPointers;
        allPointers.reserve(pointers.size());
        for (auto &pointer : pointers) {
            auto inputTx = Output(pointer, access).getSpendingTx();
            if(inputTx) {
                auto inputPointers = inputTx->getInputPointers(pointer);
                for (auto &inputPointer : inputPointers) {
                    allPointers.insert(inputPointer);
                }
            }
        }
        return allPointers
        | ranges::view::transform([&access](const InputPointer &pointer) { return Input(pointer, access); })
        | ranges::to_vector;
    }
    
    std::vector<Transaction> getTransactions(const std::vector<OutputPointer> &pointers, const DataAccess &access) {
        std::unordered_set<blocksci::Transaction> txes;
        txes.reserve(pointers.size() * 2);
        for (auto &pointer : pointers) {
            txes.insert(Transaction(pointer.txNum, access));
            auto inputTx = Output(pointer, access).getSpendingTx();
            if (inputTx) {
                txes.insert(*inputTx);
            }
        }
        return {txes.begin(), txes.end()};
    }
    
    std::vector<Transaction> getOutputTransactions(const std::vector<OutputPointer> &pointers, const DataAccess &access) {
        auto txNums = pointers | ranges::view::transform([](const OutputPointer &pointer) { return pointer.txNum; }) | ranges::to_vector;
        txNums |= ranges::action::sort | ranges::action::unique;
        return txNums | ranges::view::transform([&access](uint32_t txNum) { return Transaction(txNum, access); }) | ranges::to_vector;
    }
    
    auto flatMap() {
        return ranges::view::filter([](const auto &optional) { return static_cast<bool>(optional); })
        | ranges::view::transform([](const auto &optional) { return *optional; });
    }
    
    std::vector<Transaction> getInputTransactions(const std::vector<OutputPointer> &pointers, const DataAccess &access) {
        auto txes = pointers | ranges::view::transform([&access](const OutputPointer &pointer) { return Output(pointer, access).getSpendingTx(); }) | flatMap() | ranges::to_vector;
        txes |= ranges::action::sort | ranges::action::unique;
        return txes;
    }
}

std::ostream &operator<<(std::ostream &os, const blocksci::InputPointer &pointer) {
    os << pointer.toString();
    return os;
}

std::ostream &operator<<(std::ostream &os, const blocksci::OutputPointer &pointer) {
    os << pointer.toString();
    return os;
}

namespace std
{
    size_t hash<blocksci::InputPointer>::operator()(const blocksci::InputPointer &pointer) const {
        std::size_t seed = 41352363;
        hash_combine(seed, pointer.txNum);
        hash_combine(seed, pointer.inoutNum);
        return seed;
    }
    
    size_t hash<blocksci::OutputPointer>::operator()(const blocksci::OutputPointer &pointer) const {
        std::size_t seed = 41352363;
        hash_combine(seed, pointer.txNum);
        hash_combine(seed, pointer.inoutNum);
        return seed;
    }
}
