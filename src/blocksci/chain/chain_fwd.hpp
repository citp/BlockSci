//
//  chain_fwd.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/8/17.
//

#ifndef chain_fwd_h
#define chain_fwd_h

//#include <type_safe/strong_typedef.hpp>

namespace blocksci {
//    struct BlockHeight :
//    type_safe::strong_typedef<BlockHeight, int>,
//    type_safe::strong_typedef_op::integer_arithmetic<BlockHeight>,
//    type_safe::strong_typedef_op::relational_comparison<BlockHeight>,
//    type_safe::strong_typedef_op::equality_comparison<BlockHeight>,
//    type_safe::strong_typedef_op::mixed_relational_comparison<BlockHeight, int>,
//    type_safe::strong_typedef_op::mixed_equality_comparison<BlockHeight, int>,
//    type_safe::strong_typedef_op::mixed_division<BlockHeight, int>,
//    type_safe::strong_typedef_op::mixed_addition<BlockHeight, int>,
//    type_safe::strong_typedef_op::mixed_subtraction<BlockHeight, int>,
//    type_safe::strong_typedef_op::output_operator<BlockHeight> {
//        using strong_typedef::strong_typedef;
//    };

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
