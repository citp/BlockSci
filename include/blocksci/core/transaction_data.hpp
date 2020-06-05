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
    /** Class that brings transaction data from several files together */
    struct BLOCKSCI_EXPORT TxData {
        /** Raw transaction data, stored in chain/tx_data.dat */
        const RawTransaction *rawTx;

        /** Version field of this transaction (blockchain data), stored in chain/tx_version.dat */
        const int32_t *version;

        /** Transaction hash (256 bit), stored in chain/tx_hashes.dat */
        const uint256 *hash;

        /** Pointer to the tx-internal output number that the first input of the transaction spends. stored in chain/input_out_num.dat */
        const uint16_t *spentOutputNums;

        /** Pointer to the blockchain field <sequence number> of the transaction's first input, stored in chain/sequence.dat */
        const uint32_t *sequenceNumbers;

        TxData &operator++() {
            sequenceNumbers += rawTx->inputCount;
            spentOutputNums += rawTx->inputCount;
            version++;
            hash++;
            auto currentTxSize = sizeof(RawTransaction) + static_cast<size_t>(rawTx->inputCount) * sizeof(Inout) + static_cast<size_t>(rawTx->outputCount) * sizeof(Inout);
            rawTx = reinterpret_cast<const RawTransaction *>(reinterpret_cast<const char*>(rawTx) + currentTxSize);
            return *this;
        }
    };
}

#endif /* transaction_data_hpp */
