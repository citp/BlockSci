//
//  block_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/14/18.
//

#ifndef block_py_h
#define block_py_h

#include "method_tags.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/chain/algorithms.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>

#include <range/v3/view/any_view.hpp>

void init_uint256(pybind11::class_<blocksci::uint256> &cl);
void init_uint160(pybind11::class_<blocksci::uint160> &cl);
void init_block(pybind11::class_<blocksci::Block> &cl);

struct AddBlockMethods {
    template <typename FuncApplication>
    void operator()(FuncApplication func) {
        using namespace blocksci;
        namespace py = pybind11;
        
        func(property_tag, "txes", [](const Block &block) -> ranges::any_view<Transaction, ranges::category::random_access> {
            return block;
        }, "A range of all of the txes in the block");
        func(property_tag, "inputs", [](const Block &block) -> ranges::any_view<Input> {
            return inputs(block);
        }, "A range of all of the inputs in the block");
        func(property_tag, "outputs", [](const Block &block) -> ranges::any_view<Output> {
            return outputs(block);
        }, "A range of all of the outputs in the block");
        func(property_tag, "hash", &Block::getHash, "Hash of this block");
        func(property_tag, "version", &Block::version, "Protocol version specified in block header");
        func(property_tag, "timestamp", &Block::timestamp, "Creation timestamp specified in block header");
        func(property_tag, "time", &Block::getTime, "Datetime object created from creation timestamp");
        func(property_tag, "time_seen", &Block::getTimeSeen, "If recorded by the mempool recorder, the time that this block was first seen by your node");
        func(property_tag, "bits", &Block::bits, "Difficulty threshold specified in block header");
        func(property_tag, "nonce", &Block::nonce, "Nonce specified in block header");
        func(property_tag, "height", &Block::height, "Height of the block in the blockchain");
        func(property_tag, "coinbase_param", [](const Block &block) -> py::bytes {
            return py::bytes(block.coinbaseParam());
        }, "Data contained within the coinbase transaction of this block");
        func(property_tag, "coinbase_tx", &Block::coinbaseTx, "Return the coinbase transaction in this block");
        func(property_tag, "size_bytes", &Block::totalSize, "Returns the total size of the block in bytes");
        func(property_tag, "fee", totalFee<Block>, "The sum of the transaction fees contained in this block");
        func(property_tag, "revenue", [](const Block &block) -> int64_t {
            return totalOutputValue(block[0]);
        }, "Total reward received by the miner of this block");
        func(property_tag, "base_size", &Block::baseSize, "The size of the non-segwit data in bytes");
        func(property_tag, "total_size", &Block::totalSize, "The size all block data in bytes");
        func(property_tag, "virtual_size", &Block::virtualSize, "The weight of the block divided by 4");
        func(property_tag, "weight", &Block::weight, "Three times the base size plus the total size");
        func(property_tag, "input_value", totalInputValue<Block &>, "Returns the sum of the value of all of the inputs included in this block");
        func(property_tag, "output_value", totalOutputValue<Block &>, "Returns the sum of the value of all of the outputs included in this block");
        func(property_tag, "input_count", inputCount<Block &>, "Returns total number of inputs included in this block");
        func(property_tag, "output_count", outputCount<Block &>, "Returns total number of outputs included in this block");
        ;
    }
};

#endif /* block_py_h */
