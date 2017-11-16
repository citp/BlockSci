//
//  chain_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/8/17.
//

#ifndef chain_fwd_h
#define chain_fwd_h

namespace blocksci {
    class ChainAccess;
    
    struct RawBlock;
    class Block;
    struct RawTransaction;
    struct Transaction;
    class RawTransactionRange;
    class TransactionRange;
    struct TransactionSummary;
    struct Inout;
    class Output;
    class Input;
    
    
    
    struct InoutPointer;
    struct OutputPointer;
    struct InputPointer;
}

#endif /* chain_fwd_h */
