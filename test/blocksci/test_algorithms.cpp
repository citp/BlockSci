//
//  test_algorithms.cpp
//  blocksci_unittest
//
//  Created by Malte Möser on 4/27/20.
//

#include "unit_test.h"
#include <iostream>

namespace blocksci {

class AlgorithmsTest : public BlockSciTest {

public:

    /**
     Iterates over all outputs on the chain and returns them if boolFunc(output) returns true.
     */
    std::vector<Output> filterOutputsOnChain(std::function<bool (Output)> boolFunc) {
        std::vector<Output> outs;
        for(auto block : chain) {
            for(auto tx : block) {
                for(auto output : tx.outputs()) {
                    if(boolFunc(output)) {
                        outs.push_back(output);
                    }
                }
            }
        }
        return outs;
    }

    /**
     Iterates over all inputs on the chain and returns them if boolFunc(input) returns true.
     */
    std::vector<Input> filterInputsOnChain(std::function<bool (Input)> boolFunc) {
        std::vector<Input> ins;
        for(auto block : chain) {
            for(auto tx : block) {
                for(auto input : tx.inputs()) {
                    if(boolFunc(input)) {
                        ins.push_back(input);
                    }
                }
            }
        }
        return ins;
    }
};


TEST_F(AlgorithmsTest, CppConceptIsBlockchain) {
    ASSERT_TRUE(isBlockchain<Blockchain>);
    ASSERT_FALSE(isBlockchain<Block>);
    ASSERT_FALSE(isBlockchain<BlockRange>);
    ASSERT_FALSE(isBlockchain<Transaction>);
    ASSERT_FALSE(isBlockchain<TransactionRange>);
    ASSERT_FALSE(isBlockchain<Input>);
    ASSERT_FALSE(isBlockchain<InputRange>);
    ASSERT_FALSE(isBlockchain<Output>);
    ASSERT_FALSE(isBlockchain<OutputRange>);
}

TEST_F(AlgorithmsTest, CppConceptIsTx) {
    ASSERT_FALSE(isTx<Blockchain>);
    ASSERT_FALSE(isTx<Block>);
    ASSERT_FALSE(isTx<BlockRange>);
    ASSERT_TRUE(isTx<Transaction>);
    ASSERT_FALSE(isTx<TransactionRange>);
    ASSERT_FALSE(isTx<Input>);
    ASSERT_FALSE(isTx<InputRange>);
    ASSERT_FALSE(isTx<Output>);
    ASSERT_FALSE(isTx<OutputRange>);
}

TEST_F(AlgorithmsTest, CppConceptIsBlockRange) {
    ASSERT_TRUE(isBlockRange<Blockchain>);
    ASSERT_FALSE(isBlockRange<Block>);
    ASSERT_TRUE(isBlockRange<BlockRange>);
    ASSERT_FALSE(isBlockRange<TransactionRange>);
    ASSERT_FALSE(isBlockRange<InputRange>);
    ASSERT_FALSE(isBlockRange<OutputRange>);
}

TEST_F(AlgorithmsTest, CppConceptIsInputRange) {
    ASSERT_FALSE(isInputRange<Blockchain>);
    ASSERT_FALSE(isInputRange<Block>);
    ASSERT_FALSE(isInputRange<BlockRange>);
    ASSERT_FALSE(isInputRange<TransactionRange>);
    ASSERT_TRUE(isInputRange<InputRange>);
    ASSERT_FALSE(isInputRange<OutputRange>);
}

TEST_F(AlgorithmsTest, TxesFromChain) {
    std::vector<Transaction> transactions;
    for(auto block : chain) {
        for(auto tx : block) {
            transactions.push_back(tx);
        }
    }
    EXPECT_TRUE(transactions.size() > 0);

    auto algo_txes = txes(chain) | ranges::to_vector;

    ASSERT_EQ(transactions, algo_txes);
}

TEST_F(AlgorithmsTest, TxesFromBlock) {
    std::vector<Transaction> transactions;
    auto block = chain[123];
    for(auto tx : block) {
        transactions.push_back(tx);
    }
    EXPECT_TRUE(transactions.size() > 0);

    auto algo_txes = txes(block) | ranges::to_vector;

    ASSERT_EQ(transactions, algo_txes);
}

TEST_F(AlgorithmsTest, InputsFromBlock) {
    std::vector<Input> ins;
    auto block = chain[123];
    for(auto tx : block) {
        for(auto input : tx.inputs()) {
            ins.push_back(input);
        }
    }
    EXPECT_TRUE(ins.size() > 0);

    auto algo_inputs = inputs(block) | ranges::to_vector;

    ASSERT_EQ(ins, algo_inputs);
}

TEST_F(AlgorithmsTest, InputsFromTransaction) {
    std::vector<Input> ins;
    auto tx = chain[123][1];
    for(auto input : tx.inputs()) {
        ins.push_back(input);
    }
    EXPECT_TRUE(ins.size() > 0);

    auto algo_inputs = inputs(tx) | ranges::to_vector;

    ASSERT_EQ(ins, algo_inputs);
}

TEST_F(AlgorithmsTest, InputsFromInputRange) {
    std::vector<Input> ins;
    auto tx = chain[123][1];
    for(auto input : tx.inputs()) {
        ins.push_back(input);
    }
    EXPECT_TRUE(ins.size() > 0);

    auto algo_inputs = inputs(tx.inputs()) | ranges::to_vector;

    ASSERT_EQ(ins, algo_inputs);
}

TEST_F(AlgorithmsTest, OutputsFromBlock) {
    std::vector<Output> outs;
    auto block = chain[123];
    for(auto tx : block) {
        for(auto output : tx.outputs()) {
            outs.push_back(output);
        }
    }
    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputs(block) | ranges::to_vector;

    ASSERT_EQ(outs, algo_outputs);
}

TEST_F(AlgorithmsTest, OutputsFromTransaction) {
    std::vector<Output> outs;
    auto tx = chain[123][1];
    for(auto output : tx.outputs()) {
        outs.push_back(output);
    }
    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputs(tx) | ranges::to_vector;

    ASSERT_EQ(outs, algo_outputs);
}

TEST_F(AlgorithmsTest, OutputsFromOutputRange) {
    std::vector<Output> outs;
    auto tx = chain[123][1];
    for(auto output : tx.outputs()) {
        outs.push_back(output);
    }
    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputs(tx.outputs()) | ranges::to_vector;

    ASSERT_EQ(outs, algo_outputs);
}

// Tests that outputsUnspent returns only unspent outputs
TEST_F(AlgorithmsTest, OutputsUnspent) {
    auto filterFunc = [](Output output){return !output.isSpent();};
    auto outs = filterOutputsOnChain(filterFunc);

    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputsUnspent(chain) | ranges::to_vector;

    ASSERT_EQ(outs, algo_outputs);
}

//
TEST_F(AlgorithmsTest, OutputsSpentBeforeHeight) {
    auto filterFunc = [](Output output){return output.isSpent() && output.getSpendingTx()->getBlockHeight() < 160;};
    auto outs = filterOutputsOnChain(filterFunc);

    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputsSpentBeforeHeight(chain, 160) | ranges::to_vector;

    ASSERT_EQ(outs.size(), algo_outputs.size());
    ASSERT_EQ(outs, algo_outputs);
}

TEST_F(AlgorithmsTest, OutputsSpentAfterHeight) {
    auto filterFunc = [](Output output){return output.isSpent() && output.getSpendingTx()->getBlockHeight() >= 160;};
    auto outs = filterOutputsOnChain(filterFunc);

    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputsSpentAfterHeight(chain, 160) | ranges::to_vector;

    ASSERT_EQ(outs.size(), algo_outputs.size());
    ASSERT_EQ(outs, algo_outputs);
}

TEST_F(AlgorithmsTest, InputsCreatedBeforeHeight) {
    auto filterFunc = [](Input input){return input.getSpentTx().getBlockHeight() < 160;};
    auto ins = filterInputsOnChain(filterFunc);

    EXPECT_TRUE(ins.size() > 0);

    auto algo_inputs = inputsCreatedBeforeHeight(chain, 160) | ranges::to_vector;

    ASSERT_EQ(ins.size(), algo_inputs.size());
    ASSERT_EQ(ins, algo_inputs);
}

TEST_F(AlgorithmsTest, InputsCreatedAfterHeight) {
    auto filterFunc = [](Input input){return input.getSpentTx().getBlockHeight() >= 160;};
    auto ins = filterInputsOnChain(filterFunc);

    EXPECT_TRUE(ins.size() > 0);

    auto algo_inputs = inputsCreatedAfterHeight(chain, 160) | ranges::to_vector;

    ASSERT_EQ(ins.size(), algo_inputs.size());
    ASSERT_EQ(ins, algo_inputs);
}

TEST_F(AlgorithmsTest, OutputsSpentWithinRelativeHeight) {
    auto filterFunc = [](Output output){return output.isSpent() && (output.getSpendingTx()->getBlockHeight() - output.block().height()) < 10;};
    auto outs = filterOutputsOnChain(filterFunc);

    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputsSpentWithinRelativeHeight(chain, 10) | ranges::to_vector;

    ASSERT_EQ(outs.size(), algo_outputs.size());
    ASSERT_EQ(outs, algo_outputs);
}

TEST_F(AlgorithmsTest, OutputsSpentOutsideRelativeHeight) {
    auto filterFunc = [](Output output){return output.isSpent() && (output.getSpendingTx()->getBlockHeight() - output.block().height()) >= 10;};
    auto outs = filterOutputsOnChain(filterFunc);

    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputsSpentOutsideRelativeHeight(chain, 10) | ranges::to_vector;

    ASSERT_EQ(outs.size(), algo_outputs.size());
    ASSERT_EQ(outs, algo_outputs);
}

TEST_F(AlgorithmsTest, InputsCreatedWithinRelativeHeight) {
    auto filterFunc = [](Input input){return input.blockHeight - input.getSpentTx().getBlockHeight() < 10;};
    auto ins = filterInputsOnChain(filterFunc);

    EXPECT_TRUE(ins.size() > 0);

    auto algo_inputs = inputsCreatedWithinRelativeHeight(chain, 10) | ranges::to_vector;

    ASSERT_EQ(ins.size(), algo_inputs.size());
    ASSERT_EQ(ins, algo_inputs);
}

TEST_F(AlgorithmsTest, InputsCreatedOutsideRelativeHeight) {
    auto filterFunc = [](Input input){return input.blockHeight - input.getSpentTx().getBlockHeight() >= 10;};
    auto ins = filterInputsOnChain(filterFunc);

    EXPECT_TRUE(ins.size() > 0);

    auto algo_inputs = inputsCreatedOutsideRelativeHeight(chain, 10) | ranges::to_vector;

    ASSERT_EQ(ins.size(), algo_inputs.size());
    ASSERT_EQ(ins, algo_inputs);
}

TEST_F(AlgorithmsTest, OutputsOfType) {
    auto filterFunc = [](Output output){return output.getType() == AddressType::PUBKEYHASH;};
    auto outs = filterOutputsOnChain(filterFunc);

    EXPECT_TRUE(outs.size() > 0);

    auto algo_outputs = outputsOfType(chain, AddressType::PUBKEYHASH) | ranges::to_vector;

    ASSERT_EQ(outs.size(), algo_outputs.size());
    ASSERT_EQ(outs, algo_outputs);
}

TEST_F(AlgorithmsTest, InputsOfType) {
    auto filterFunc = [](Input input){return input.getType() == AddressType::SCRIPTHASH;};
    auto ins = filterInputsOnChain(filterFunc);

    EXPECT_TRUE(ins.size() > 0);

    auto algo_inputs = inputsOfType(chain, AddressType::SCRIPTHASH) | ranges::to_vector;

    ASSERT_EQ(ins.size(), algo_inputs.size());
    ASSERT_EQ(ins, algo_inputs);
}

TEST_F(AlgorithmsTest, InputCount) {
    uint64_t count = 0;
    for(auto block : chain) {
        for(auto tx : block) {
            count += tx.inputCount();
        }
    }
    EXPECT_TRUE(count > 0);

    auto algo_count = inputCount(chain);

    ASSERT_EQ(count, algo_count);
}

TEST_F(AlgorithmsTest, OutputCount) {
    uint64_t count = 0;
    for(auto block : chain) {
        for(auto tx : block) {
            count += tx.outputCount();
        }
    }
    EXPECT_TRUE(count > 0);

    auto algo_count = outputCount(chain);

    ASSERT_EQ(count, algo_count);
}

TEST_F(AlgorithmsTest, TotalInputValue) {
    int64_t amount = 0;
    for(auto block : chain) {
        for(auto tx : block) {
            for(auto input : tx.inputs()) {
                amount += input.getValue();
            }
        }
    }
    EXPECT_TRUE(amount > 0);

    auto algo_amount = totalInputValue(chain);

    ASSERT_EQ(amount, algo_amount);
}

TEST_F(AlgorithmsTest, TotalOutputValue) {
    int64_t amount = 0;
    for(auto block : chain) {
        for(auto tx : block) {
            for(auto output : tx.outputs()) {
                amount += output.getValue();
            }
        }
    }
    EXPECT_TRUE(amount > 0);

    auto algo_amount = totalOutputValue(chain);

    ASSERT_EQ(amount, algo_amount);
}

TEST_F(AlgorithmsTest, Fee) {
    for(auto block : chain) {
        for(auto tx : block) {
            ASSERT_EQ(fee(tx), tx.fee());
        }
    }
}


TEST_F(AlgorithmsTest, FeeLessThan) {
    std::vector<Transaction> txes;
    for(auto block : chain) {
        for(auto tx : block) {
            if(tx.fee() < 1) {
                txes.push_back(tx);
            }
        }
    }
    EXPECT_TRUE(txes.size() > 0);

    auto algo_txes = feeLessThan(chain, 1) | ranges::to_vector;

    ASSERT_EQ(txes.size(), algo_txes.size());
    ASSERT_EQ(txes, algo_txes);
}

TEST_F(AlgorithmsTest, FeeGreaterThan) {
    std::vector<Transaction> txes;
    for(auto block : chain) {
        for(auto tx : block) {
            if(tx.fee() >= 1) {
                txes.push_back(tx);
            }
        }
    }
    EXPECT_TRUE(txes.size() > 0);

    auto algo_txes = feeGreaterThan(chain, 1) | ranges::to_vector;

    ASSERT_EQ(txes.size(), algo_txes.size());
    ASSERT_EQ(txes, algo_txes);
}

TEST_F(AlgorithmsTest, Fees) {
    std::vector<int64_t> txFees;
    for(auto block : chain) {
        for(auto tx : block) {
            txFees.push_back(tx.fee());
        }
    }
    EXPECT_TRUE(txFees.size() > 0);

    auto algo_fees = fees(chain) | ranges::to_vector;

    ASSERT_EQ(txFees.size(), algo_fees.size());
    ASSERT_EQ(txFees, algo_fees);
}

TEST_F(AlgorithmsTest, TotalFees) {
    int64_t amount = 0;
    for(auto block : chain) {
        for(auto tx : block) {
            amount += tx.fee();
        }
    }
    EXPECT_TRUE(amount > 0);

    auto algo_amount = totalFee(chain);

    ASSERT_EQ(amount, algo_amount);
}

}  // namespace blocksci

