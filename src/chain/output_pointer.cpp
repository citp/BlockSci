//
//  inout_pointer.cpp
//  blocksci
//
//  Created by Harry Kalodner on 9/4/17.
//
//

#include <blocksci/chain/output_pointer.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/range_util.hpp>

#include <internal/chain_access.hpp>
#include <internal/data_access.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/transform.hpp>

#include <sstream>
#include <unordered_set>

namespace blocksci {
    
    std::string OutputPointer::toString() const {
        std::stringstream ss;
        ss << "OutputPointer(tx_index_from=" << txNum << ", output_index_from=" << inoutNum << ")";
        return ss.str();
    }
    
    std::vector<Input> getInputs(const std::vector<OutputPointer> &pointers, DataAccess &access) {
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
        | ranges::views::transform([&access](const InputPointer &pointer) { return Input(pointer, access); })
        | ranges::to_vector;
    }
    
    std::vector<Transaction> getTransactions(const std::vector<OutputPointer> &pointers, DataAccess &access) {
        std::unordered_set<blocksci::Transaction> txes;
        txes.reserve(pointers.size() * 2);
        for (auto &pointer : pointers) {
            txes.insert(Transaction(pointer.txNum, access.getChain().getBlockHeight(pointer.txNum), access));
            auto inputTx = Output(pointer, access).getSpendingTx();
            if (inputTx) {
                txes.insert(*inputTx);
            }
        }
        return {txes.begin(), txes.end()};
    }
    
    std::vector<Transaction> getOutputTransactions(const std::vector<OutputPointer> &pointers, DataAccess &access) {
        auto txNums = pointers | ranges::views::transform([](const OutputPointer &pointer) { return pointer.txNum; }) | ranges::to_vector;
        txNums |= ranges::action::sort | ranges::action::unique;
        return txNums | ranges::views::transform([&access](uint32_t txNum) { return Transaction(txNum, access.getChain().getBlockHeight(txNum), access); }) | ranges::to_vector;
    }
    
    std::vector<Transaction> getInputTransactions(const std::vector<OutputPointer> &pointers, DataAccess &access) {
        auto txes = pointers | ranges::views::transform([&access](const OutputPointer &pointer) { return Output(pointer, access).getSpendingTx(); }) | flatMapOptionals() | ranges::to_vector;
        txes |= ranges::action::sort | ranges::action::unique;
        return txes;
    }
} // namespace blocksci

std::ostream &operator<<(std::ostream &os, const blocksci::OutputPointer &pointer) {
    os << pointer.toString();
    return os;
}
