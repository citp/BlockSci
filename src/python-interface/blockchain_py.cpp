//
//  blockchain_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/scripts/address_index.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;

using namespace blocksci;

py::list segmentChainPy(Blockchain &chain, uint32_t start, uint32_t stop, uint32_t segmentCount) {
    auto segments = segmentChain(chain, start, stop, segmentCount);
    
    py::list segmentsPython;
    for (auto &segment : segments) {
        py::list segmentPython;
        for (const auto &block : segment) {
            segmentPython.append(block);
        }
        segmentsPython.append(segmentPython);
    }
    return segmentsPython;
}

py::object reduceOverTransactionsPython(Blockchain &chain, uint32_t start, uint32_t stop, py::object initial, py::function extractPy, py::function combinePy) {
    
    auto extract = [extractPy](const Transaction &tx) {
        return extractPy(tx);
    };
    
    auto combine = [combinePy](py::object &res1, py::object res2) {
        return combinePy(res1, res2);
    };
    
    chain.mapReduceTransactions(start, stop, extract, combine, initial);
    return initial;
}

void init_blockchain(py::module &m) {
    py::class_<Blockchain>(m, "Blockchain", "Class representing the blockchain")
    .def(py::init<std::string>())
    .def("reduce_over_transactions", reduceOverTransactionsPython)
    .def("segment", segmentChainPy)
    .def("get_coinjoin_transactions", getCoinjoinTransactions)
    .def("get_possible_coinjoin_transactions", getPossibleCoinjoinTransactions)
    .def("get_transactions_with_output", getTransactionIncludingOutput)
    .def("get_deanon_txes", getDeanonTxes)
    .def("get_change_over_txes", getChangeOverTxes)
    .def("get_keyset_change_txes", getKeysetChangeTxes)
    .def("get_address_index", &Blockchain::getAddressIndex)
    .def("get_matching_txes", [](const Blockchain &chain, uint32_t startBlock, uint32_t endBlock, py::function testFunc) {
        std::vector<Transaction> txes;
        for (auto tx : chain.iterateTransactions(startBlock, endBlock)) {
            if (testFunc(tx)) {
                txes.push_back(tx);
            }
        }
        return txes;
    })
    .def("__len__", &Blockchain::size)
    /// Optional sequence protocol operations
    .def("__iter__", [](const Blockchain &chain) { return py::make_iterator(chain.begin(), chain.end()); },
         py::keep_alive<0, 1>() /* Essential: keep object alive while iterator exists */)
    .def("__getitem__", [](const Blockchain &chain, int64_t i) {
        if (i < 0) {
            i = (chain.size() + i) % chain.size();
        }
        uint64_t posIndex = static_cast<uint64_t>(i);
        if (posIndex >= chain.size()) {
            throw py::index_error();
        }
        return chain[i];
    })
    .def("__getitem__", [](const Blockchain &chain, py::slice slice) -> py::list {
        size_t start, stop, step, slicelength;
        if (!slice.compute(chain.size(), &start, &stop, &step, &slicelength))
            throw py::error_already_set();
        py::list blockList;
        for (size_t i=0; i<slicelength; ++i) {
            blockList.append(chain[start]);
            start += step;
        }
        return blockList;
    })
    ;
}
