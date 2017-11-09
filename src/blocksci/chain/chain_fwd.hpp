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
    
    struct Block;
    struct Transaction;
    class TransactionIterator;
    struct TransactionSummary;
    struct Inout;
    struct Output;
    struct Input;
    
    struct InoutPointer;
    struct OutputPointer;
    struct InputPointer;
}

#endif /* chain_fwd_h */
