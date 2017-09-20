//
//  transaction_summary.cpp
//  blocksci
//
//  Created by Harry Kalodner on 8/10/17.
//
//

#include "transaction_summary.hpp"
#include "transaction.hpp"
#include "output.hpp"

namespace blocksci {
    
    TransactionSummary::TransactionSummary(uint64_t totalInputs_, uint64_t totalOutputs_, uint64_t totalSize_, uint64_t totalCount_, uint64_t totalOutputValue_) : totalInputs(totalInputs_), totalOutputs(totalOutputs_), totalSize(totalSize_), totalCount(totalCount_), totalOutputValue(totalOutputValue_) {}
    
    TransactionSummary::TransactionSummary() : TransactionSummary{0,0,0,0,0} {}
    
    TransactionSummary TransactionSummary::operator+(const TransactionSummary &other) const {
        return {totalInputs + other.totalInputs, totalOutputs + other.totalOutputs, totalSize + other.totalSize, totalCount + other.totalCount, totalOutputValue + other.totalOutputValue};
    }
    
    TransactionSummary TransactionSummary::operator+(const Transaction &other) const {
        TransactionSummary a{*this};
        a += other;
        return a;
    }
    
    TransactionSummary &TransactionSummary::operator+=(const Transaction &other) {
        totalInputs += other.inputCount();
        totalOutputs += other.outputCount();
        totalSize += other.sizeBytes();
        totalCount += 1;
        for (auto &output : other.outputs()) {
            totalOutputValue += output.getValue();
        }
        return *this;
    }
    
    TransactionSummary &TransactionSummary::operator+=(const TransactionSummary &other) {
        totalInputs += other.totalInputs;
        totalOutputs += other.totalOutputs;
        totalSize += other.totalSize;
        totalCount += other.totalCount;
        totalOutputValue += other.totalOutputValue;
        return *this;
    }
    
    double TransactionSummary::averageInputs() const {
        return static_cast<double>(totalInputs) / static_cast<double>(totalCount);
    }
    
    double TransactionSummary::averageOutputs() const {
        return static_cast<double>(totalOutputs) / static_cast<double>(totalCount);
    }
    
    double TransactionSummary::averageSize() const {
        return static_cast<double>(totalSize) / static_cast<double>(totalCount);
    }
    
    double TransactionSummary::averageOutputValue() const {
        return static_cast<double>(totalOutputValue) / static_cast<double>(totalCount);
    }
}
