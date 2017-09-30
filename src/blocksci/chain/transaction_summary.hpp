//
//  transaction_summary.hpp
//  blocksci
//
//  Created by Harry Kalodner on 8/10/17.
//
//

#ifndef transaction_summary_hpp
#define transaction_summary_hpp

#include <cstdint>

namespace blocksci {
    
    struct Transaction;
    struct RawTransaction;
    
    struct TransactionSummary {
        uint64_t totalInputs;
        uint64_t totalOutputs;
        uint64_t totalSize;
        uint64_t totalCount;
        uint64_t totalOutputValue;
        
        TransactionSummary(uint64_t totalInputs, uint64_t totalOutputs, uint64_t totalSize, uint64_t totalCount, uint64_t totalOutputValue);
        TransactionSummary();
        
        TransactionSummary operator+(const TransactionSummary &other) const;
        TransactionSummary operator+(const Transaction &other) const;
        TransactionSummary &operator+=(const Transaction &other);
        TransactionSummary &operator+=(const TransactionSummary &other);
        double averageInputs() const;
        double averageOutputs() const;
        double averageSize() const;
        double averageOutputValue() const;
    };
}

#endif /* transaction_summary_hpp */
