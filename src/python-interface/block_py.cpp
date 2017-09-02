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

py::object reduceSpentOutputsPython(const Transaction::input_range &col, py::function callable, py::object initial);
py::object reduceOutputsPython(const Transaction::output_range &col, py::function callable, py::object initial);

py::object reduceOverTxesPython(Block &block, py::function callable, py::object initial) {
    auto accumulated = initial;
    for (auto tx : block) {
        accumulated = callable(accumulated, tx.txNum, tx.blockHeight, tx.sizeBytes(), tx.outputCount(), tx.inputCount());
    }
    return accumulated;
}

py::object reduceOverTxInsPython(Block &block, py::function callable, py::object initial) {
    auto accumulated = initial;
    for (auto tx : block) {
        accumulated = reduceSpentOutputsPython(tx.inputs(), callable, accumulated);
    }
    return accumulated;
}

py::object reduceOverTxOutsPython(Block &block, py::function callable, py::object initial) {
    auto accumulated = initial;
    for (auto tx : block) {
        accumulated = reduceOutputsPython(tx.outputs(), callable, accumulated);
    }
    return accumulated;
}

namespace pybind11 { namespace detail {

// This is for casting times on the system clock into datetime.datetime instances
template<> class type_caster<boost::posix_time::ptime> {
public:
    static handle cast(const boost::posix_time::ptime &src, return_value_policy /* policy */, handle /* parent */) {
        
        // Lazy initialise the PyDateTime import
        if (!PyDateTimeAPI) { PyDateTime_IMPORT; }
        
        auto date = src.date();
        auto time = src.time_of_day();
        
        return PyDateTime_FromDateAndTime(date.year(),
                                          date.month(),
                                          date.day(),
                                          time.hours(),
                                          time.minutes(),
                                          time.seconds(),
                                          time.total_microseconds());
    }
    PYBIND11_TYPE_CASTER(boost::posix_time::ptime, _("datetime.datetime"));
};
}}

void init_block(py::module &m) {
    py::class_<uint256>(m, "uint256")
    .def("__repr__", &uint256::GetHex)
    .def("__getstate__", [](const uint256 &hash) {
        return py::make_tuple(hash.GetHex());
    })
    .def("__setstate__", [](uint256 &p, py::tuple t) {
        if (t.size() != 1)
            throw std::runtime_error("Invalid state!");
        
        p = uint256S(t[0].cast<std::string>());
    })
    ;
    
    py::class_<uint160>(m, "uint160")
    .def("__repr__", &uint160::GetHex)
    .def("__getstate__", [](const uint160 &hash) {
        return py::make_tuple(hash.GetHex());
    })
    .def("__setstate__", [](uint160 &p, py::tuple t) {
        if (t.size() != 1)
            throw std::runtime_error("Invalid state!");
        
        p = uint160S(t[0].cast<std::string>());
    })
    ;
    
    py::class_<Block>(m, "Block", "Class representing a block in the blockchain")
    .def(py::init<uint32_t, uint32_t, uint32_t, uint256, int32_t, uint32_t, uint32_t, uint32_t, uint64_t>())
    .def("__getstate__", [](const Block &block) {
        return py::make_tuple(block.firstTxIndex, block.numTxes, block.height, block.hash, block.version, block.timestamp, block.bits, block.nonce, block.coinbaseOffset);
    })
    .def("__setstate__", [](Block &p, py::tuple t) {
        if (t.size() != 10)
            throw std::runtime_error("Invalid state!");
        
        /* Invoke the in-place constructor. Note that this is needed even
         when the object just has a trivial default constructor */
        new (&p) Block(t[0].cast<uint32_t>(), t[1].cast<uint32_t>(), t[2].cast<uint32_t>(), t[3].cast<uint256>(), t[4].cast<uint32_t>(), t[5].cast<uint32_t>(), t[6].cast<uint32_t>(), t[7].cast<uint32_t>(), t[8].cast<uint64_t>());
    })
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
    .def_property_readonly("header_hash", &Block::getHeaderHash)
    .def_readonly("version", &Block::version, "Protocol version specified in block header")
    .def_readonly("timestamp", &Block::timestamp, "Creation timestamp specified in block header")
    .def("get_time", &Block::getTime)
    .def_readonly("bits", &Block::bits, "Difficulty threshold specified in block header")
    .def_readonly("nonce", &Block::bits, "Nonce specified in block header")
    .def_readonly("numTxes", &Block::numTxes, "Number of transactions contained in this block")
    .def_readonly("height", &Block::height, "Height of the block in the blockchain")
    .def_property_readonly("coinbase_param", [](const Block &block) {
        return py::bytes(block.coinbaseParam());
    }, "Data contained within the coinbase transaction of this block")
    .def("coinbase_tx", static_cast<Transaction(Block::*)() const>(&Block::coinbaseTx), "Return the coinbase transcation in this block")
    .def("reduce_over_txes", reduceOverTxesPython)
    .def("reduce_over_txins", reduceOverTxInsPython)
    .def("reduce_over_txouts", reduceOverTxOutsPython)
    .def("all_fees", py::overload_cast<const Block &>(allFees), "Return a list of the fees in this block")
    .def("all_fee_ratios", py::overload_cast<const Block &>(feesPerByte), "Return a list of fees per byte in this block")
    .def("size_bytes", py::overload_cast<const Block &>(sizeBytes), "Returns the total size of the block in bytes")
    .def("total_in", py::overload_cast<const Block &>(totalIn))
    .def("total_out", py::overload_cast<const Block &>(totalOut))
    .def("total_out_after_height", py::overload_cast<const Block &, uint32_t>(totalOutAfterHeight))
    .def("unspent_outputs", py::overload_cast<const Block &>(getUnspentOutputs))
    .def("outputs_spent_by_height", py::overload_cast<const Block &, uint32_t>(getOutputsSpentByHeight))
    .def("total_spent_of_age", py::overload_cast<const Block &, uint32_t>(getTotalSpentOfAge))
    .def("total_spent_of_ages", py::overload_cast<const Block &, uint32_t>(getTotalSpentOfAges))
    ;
}
