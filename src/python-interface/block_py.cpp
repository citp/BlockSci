//
//  block_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//


#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/transaction_iterator.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/chrono.h>

#include <boost/range/numeric.hpp>

namespace py = pybind11;

using namespace blocksci;

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
    
    py::class_<Block>(m, "Block", "Class representing a block in the blockchain")
    .def(py::init<uint32_t, uint32_t, uint32_t, uint256, int32_t, uint32_t, uint32_t, uint32_t, uint64_t, double>())
    .def(py::pickle(
        [](const Block &block) {
            return py::make_tuple(block.firstTxIndex, block.numTxes, block.height, block.hash, block.version, block.timestamp, block.bits, block.nonce, block.coinbaseOffset, block.difficulty);
        },
        [](py::tuple t) {
            if (t.size() != 9)
                throw std::runtime_error("Invalid state!");

            return Block(t[0].cast<uint32_t>(), t[1].cast<uint32_t>(), t[2].cast<uint32_t>(), t[3].cast<uint256>(), t[4].cast<uint32_t>(), t[5].cast<uint32_t>(), t[6].cast<uint32_t>(), t[7].cast<uint32_t>(), t[8].cast<uint64_t>(), t[9].cast<double>());
        }
    ))
    .def("__repr__", &Block::getString)
    .def("__len__", &Block::size)
    /// Optional sequence protocol operations
    .def("__iter__", [](const Block &block) { return py::make_iterator(block.begin(), block.end()); },
         py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
    .def("__getitem__", [](const Block &block, int64_t i) {
        while (i < 0) {
            i = block.size() - i;
        }
        uint64_t posIndex = static_cast<uint64_t>(i);
        if (posIndex >= block.size())
            throw py::index_error();
        return block[i];
    })
    .def("__getitem__", [](const Block &block, py::slice slice) -> py::list {
        size_t start, stop, step, slicelength;
        if (!slice.compute(block.size(), &start, &stop, &step, &slicelength))
            throw py::error_already_set();
        py::list txList;
        for (size_t i=0; i<slicelength; ++i) {
            txList.append(block[start]);
            start += step;
        }
        return txList;
    })
    .def_property_readonly("header_hash", &Block::getHeaderHash, "Hash of this block")
    .def_readonly("version", &Block::version, "Protocol version specified in block header")
    .def_readonly("timestamp", &Block::timestamp, "Creation timestamp specified in block header")
    .def_property_readonly("time", &Block::getTime, "Datetime object created from creation timestamp")
    .def_readonly("bits", &Block::bits, "Difficulty threshold specified in block header")
    .def_readonly("nonce", &Block::bits, "Nonce specified in block header")
    .def_readonly("height", &Block::height, "Height of the block in the blockchain")
    .def_readonly("difficulty", &Block::difficulty, "Difficulty of the block")
    .def_property_readonly("coinbase_param", [](const Block &block) {
        return py::bytes(block.coinbaseParam());
    }, "Data contained within the coinbase transaction of this block")
    .def_property_readonly("coinbase_tx", py::overload_cast<>(&Block::coinbaseTx, py::const_), "Return the coinbase transcation in this block")
    .def_property_readonly("fees", py::overload_cast<const Block &>(fees), "Return a list of the fees in this block")
    .def_property_readonly("fees_per_byte", py::overload_cast<const Block &>(feesPerByte), "Return a list of fees per byte in this block")
    .def_property_readonly("size_bytes", py::overload_cast<const Block &>(sizeBytes), "Returns the total size of the block in bytes")
    .def_property_readonly("value_in", py::overload_cast<const Block &>(totalIn), "Returns the sum of the value of all of the inputs included in this block")
    .def_property_readonly("value_out", py::overload_cast<const Block &>(totalOut), "Returns the sum of the value of all of the outputs included in this block")
    .def_property_readonly("unspent_outputs", py::overload_cast<const Block &>(getUnspentOutputs), "Returns a list of all of the outputs in this block that are still unspent")
    .def("value_out_after_height", py::overload_cast<const Block &, uint32_t>(totalOutAfterHeight), "Returns the sum of all of the outputs that were not spent until after the given height")
    .def("outputs_spent_by_height", py::overload_cast<const Block &, uint32_t>(getOutputsSpentByHeight), "Returns a list of all of the outputs that were spent by the given height")
    .def("total_spent_of_age", py::overload_cast<const Block &, uint32_t>(getTotalSpentOfAge), "Returns the sum of all the outputs in the block that were spent within the given number of blocks")
    .def("total_spent_of_ages", py::overload_cast<const Block &, uint32_t>(getTotalSpentOfAges), "Returns a list of sum of all the outputs in the block that were spent within a certain of blocks, up to the max age given")
    .def("net_address_type_value", py::overload_cast<const Block &>(netAddressTypeValue), "Returns a set of the net change in the utxo pool after this block split up by address type")
    ;
}
