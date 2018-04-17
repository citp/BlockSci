//
//  transaction_summary.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/10/17.
//
//

#ifndef transaction_summary_hpp
#define transaction_summary_hpp

#include "transaction.hpp"

namespace blocksci {
    struct TransactionSummary {
        uint64_t totalInputs;
        uint64_t totalOutputs;
        uint64_t totalSize;
        uint64_t totalCount;
        int64_t totalOutputValue;
        
        TransactionSummary(uint64_t totalInputs_, uint64_t totalOutputs_, uint64_t totalSize_, uint64_t totalCount_, int64_t totalOutputValue_) : totalInputs(totalInputs_), totalOutputs(totalOutputs_), totalSize(totalSize_), totalCount(totalCount_), totalOutputValue(totalOutputValue_) {}
        TransactionSummary() : TransactionSummary{0,0,0,0,0} {}
        
        TransactionSummary operator+(const TransactionSummary &other) const {
            return {totalInputs + other.totalInputs, totalOutputs + other.totalOutputs, totalSize + other.totalSize, totalCount + other.totalCount, totalOutputValue + other.totalOutputValue};
        }
        
        TransactionSummary operator+(const Transaction &other) const {
            TransactionSummary a{*this};
            a += other;
            return a;
        }
        
        TransactionSummary &operator+=(const Transaction &other) {
            totalInputs += other.inputCount();
            totalOutputs += other.outputCount();
            totalSize += other.sizeBytes();
            totalCount += 1;
            for (auto output : other.outputs()) {
                totalOutputValue += output.getValue();
            }
            return *this;
        }
        
        TransactionSummary &operator+=(const TransactionSummary &other) {
            totalInputs += other.totalInputs;
            totalOutputs += other.totalOutputs;
            totalSize += other.totalSize;
            totalCount += other.totalCount;
            totalOutputValue += other.totalOutputValue;
            return *this;
        }
        
        double averageInputs() const {
            return static_cast<double>(totalInputs) / static_cast<double>(totalCount);
        }
        
        double averageOutputs() const {
            return static_cast<double>(totalOutputs) / static_cast<double>(totalCount);
        }
        
        double averageSize() const {
            return static_cast<double>(totalSize) / static_cast<double>(totalCount);
        }
        
        double averageOutputValue() const {
            return static_cast<double>(totalOutputValue) / static_cast<double>(totalCount);
        }
    };
} // namespace blocksci

#endif /* transaction_summary_hpp */
