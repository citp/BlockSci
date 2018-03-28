//
//  chain_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/8/17.
//

#ifndef chain_fwd_h
#define chain_fwd_h

namespace blocksci {
    using BlockHeight = int;
    
    class ChainAccess;
    
    struct RawBlock;
    struct RawTransaction;
    struct Inout;
    
    class Blockchain;
    class Block;
    class Transaction;
    class Output;
    class Input;
    
    class RawTransactionRange;
    class TransactionRange;
    struct TransactionSummary;
    
    struct InoutPointer;
    struct OutputPointer;
    struct InputPointer;
}

#endif /* chain_fwd_h */
