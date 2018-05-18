//
//  transaction_data.hpp
//  blocksci
//
//  Created by Harry Kalodner on 1/4/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef transaction_data_hpp
#define transaction_data_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/core/bitcoin_uint256.hpp>
#include <blocksci/core/raw_transaction.hpp>

#include <cstdint>

namespace blocksci {
    struct BLOCKSCI_EXPORT TxData {
        const RawTransaction *rawTx;
        const int32_t *version;
        const uint256 *hash;
        const uint32_t *sequenceNumbers;

        TxData &operator++() {
            sequenceNumbers += rawTx->inputCount;
            version++;
            hash++;
            auto currentTxSize = sizeof(RawTransaction) + static_cast<size_t>(rawTx->inputCount) * sizeof(Inout) + static_cast<size_t>(rawTx->outputCount) * sizeof(Inout);
            rawTx = reinterpret_cast<const RawTransaction *>(reinterpret_cast<const char*>(rawTx) + currentTxSize);
            return *this;
        }
    };
}

#endif /* transaction_data_hpp */
