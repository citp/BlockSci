//
//  block_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "block_py.hpp"
#include "ranges_py.hpp"
#include "optional_py.hpp"

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/chrono.h>

#include <range/v3/view/any_view.hpp>

namespace py = pybind11;

using namespace blocksci;

template <typename T>
auto addBlockRange(py::module &m, const std::string &name) {
    auto cl = addRangeClass<T>(m, name);
    addBlockMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each block: " << docstring;
        return strdup(ss.str().c_str());
    });
    addBlockRangeMethods(cl, [](auto &range, auto func) {
        return func(range);
    });
    return cl;
}

template <typename T>
auto addOptionalBlockRange(py::module &m, const std::string &name) {
    auto cl = addOptionalRangeClass<T>(m, name);
    addBlockMethods(cl, [](auto func) {
        return applyMethodsToRange<T>(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each block: " << docstring;
        return strdup(ss.str().c_str());
    });
    addBlockRangeMethods(cl, [](auto &range, auto func) {
        return func(range | flatMapOptionals);
    });
    return cl;
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
    .def("__repr__", &Block::toString)
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
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    addBlockRange<ranges::any_view<Block>>(m, "AnyBlockRange");
    addBlockRange<ranges::any_view<Block, ranges::category::random_access>>(m, "BlockRange");
    addOptionalBlockRange<ranges::any_view<ranges::optional<Block>>>(m, "AnyOptionaBlockRange");
    addOptionalBlockRange<ranges::any_view<ranges::optional<Block>, ranges::category::random_access>>(m, "OptionaBlockRange");
}
