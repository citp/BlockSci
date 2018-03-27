//
//  block_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "ranges_py.hpp"
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>
#include <blocksci/util/data_access.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/chrono.h>

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/stride.hpp>
#include <range/v3/view/slice.hpp>

namespace py = pybind11;

using namespace blocksci;

template <typename Class, typename FuncApplication>
void addBlockMethods(Class &cl, FuncApplication func) {
    cl
    .def_property_readonly("hash", func([](const Block &block) {
        return block.getHeaderHash();
    }), "Hash of this block")
    .def_property_readonly("version", func([](const Block &block) {
        return block.version();
    }), "Protocol version specified in block header")
    .def_property_readonly("timestamp", func([](const Block &block) {
        return block.timestamp();
    }), "Creation timestamp specified in block header")
    .def_property_readonly("time", func([](const Block &block) {
        return block.getTime();
    }), "Datetime object created from creation timestamp")
    .def_property_readonly("bits", func([](const Block &block) {
        return block.bits();
    }), "Difficulty threshold specified in block header")
    .def_property_readonly("nonce", func([](const Block &block) {
        return block.nonce();
    }), "Nonce specified in block header")
    .def_property_readonly("height", func([](const Block &block) {
        return block.height();
    }), "Height of the block in the blockchain")
    .def_property_readonly("coinbase_param", func([](const Block &block) {
        return py::bytes(block.coinbaseParam());
    }), "Data contained within the coinbase transaction of this block")
    .def_property_readonly("coinbase_tx", func([](const Block &block) {
        return block.coinbaseTx();
    }), "Return the coinbase transaction in this block")
    .def_property_readonly("size_bytes", func([](const Block &block) {
        return block.totalSize();
    }), "Returns the total size of the block in bytes")
    .def_property_readonly("fee", func([](const Block &block) {
        return totalFee(block);
    }), "The sum of the transaction fees contained in this block")
    .def_property_readonly("revenue", func([](const Block &block) {
        return totalOutputValue(block[0]);
    }), "Total reward received by the miner of this block")
    .def_property_readonly("base_size", func([](const Block &block) {
        return block.baseSize();
    }), "The size of the non-segwit data in bytes")
    .def_property_readonly("total_size", func([](const Block &block) {
        return block.totalSize();
    }), "The size all block data in bytes")
    .def_property_readonly("virtual_size", func([](const Block &block) {
        return block.virtualSize();
    }), "The weight of the block divided by 4")
    .def_property_readonly("weight", func([](const Block &block) {
        return block.weight();
    }), "Three times the base size plus the total size")
    .def_property_readonly("input_value", func([](const Block &block) {
        return totalInputValue(block);
    }), "Returns the sum of the value of all of the inputs included in this block")
    .def_property_readonly("output_value", func([](const Block &block) {
        return totalOutputValue(block);
    }), "Returns the sum of the value of all of the outputs included in this block")
    .def_property_readonly("input_count", func([](const Block &block) {
        return inputCount(block);
    }), "Returns total number of inputs included in this block")
    .def_property_readonly("output_count", func([](const Block &block) {
        return outputCount(block);
    }), "Returns total number of outputs included in this block")
    ;
}

void init_block(py::module &m) {
    py::class_<uint256>(m, "uint256")
    .def("__repr__", &uint256::GetHex)
    .def(py::pickle(
        [](const uint256 &hash) {
            return py::make_tuple(hash.GetHex());
        },
        [](py::tuple t) {
            if (t.size() != 1) {
                throw std::runtime_error("Invalid state!");
            }
            return uint256S(t[0].cast<std::string>());
        }
    ))
    ;
    
    py::class_<uint160>(m, "uint160")
    .def("__repr__", &uint160::GetHex)
    .def(py::pickle(
        [](const uint160 &hash) {
            return py::make_tuple(hash.GetHex());
        },
        [](py::tuple t) {
            if (t.size() != 1) {
                throw std::runtime_error("Invalid state!");
            }
            return uint160S(t[0].cast<std::string>());
        }
    ))
    ;
    
    auto blockCl = addRangeClass<Block>(m, "Block", "Class representing a block in the blockchain");
    blockCl
    .def("__repr__", &Block::getString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &Block::getAccess, py::return_value_policy::reference)
    .def_property_readonly("txes", [](const Block &block) -> ranges::any_view<Transaction, ranges::category::random_access> {
        return block;
    }, R"docstring(
         Returns a range of all of the txes in the block.

         :returns: AnyTxRange
         )docstring")
    .def_property_readonly("inputs", [](const Block &block) -> ranges::any_view<Input> {
        return inputs(block);
    }, R"docstring(
         Returns a range of all of the inputs in the block.

         :returns: AnyInputRange
         )docstring")

    .def_property_readonly("outputs", [](const Block &block) -> ranges::any_view<Output> {
        return outputs(block);
    }, R"docstring(
         Returns a range of all of the outputs in the block.

         :returns: AnyOutputRange
         )docstring")
    .def_property_readonly("next_block", &Block::nextBlock, "Returns the block which follows this one in the chain")
    .def_property_readonly("prev_block", &Block::prevBlock, "Returns the block which comes before this one in the chain")
    .def("total_spent_of_ages", py::overload_cast<const Block &, blocksci::BlockHeight>(getTotalSpentOfAges), "Returns a list of sum of all the outputs in the block that were spent within a certain of blocks, up to the max age given")
    .def("net_address_type_value", py::overload_cast<const Block &>(netAddressTypeValue), "Returns a set of the net change in the utxo pool after this block split up by address type")
    .def("net_full_type_value", py::overload_cast<const Block &>(netFullTypeValue), "Returns a set of the net change in the utxo pool after this block split up by full type")
    ;

    addBlockMethods(blockCl, [](auto func) {
        return [=](Block &block) {
            return func(block);
        };
    });
    
    auto blockRangeCl = addRangeClass<ranges::any_view<Block, ranges::category::random_access>>(m, "BlockRange");
    addBlockMethods(blockRangeCl, [](auto func) {
        return applyMethodsToRange<ranges::any_view<Block, ranges::category::random_access>>(func);
    });
}
