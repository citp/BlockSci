//
//  chain_algorithms.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 11/21/17.
//

#ifndef python_chain_algorithms_hpp
#define python_chain_algorithms_hpp

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/output.hpp>

#include <pybind11/pybind11.h>

#include <range/v3/view/any_view.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/algorithm/all_of.hpp>

#include <stdio.h>

template<typename Class>
void addOutputAlgorithms(pybind11::module &m, Class & cl) {
    using Range = typename Class::type;
    m.def("outputs_spent_within_relative_height",
          [](Range &range, int difference) -> ranges::any_view<blocksci::Output> {
              return outputsSpentWithinRelativeHeight(range, difference);
          }, "Returns a list of all of the outputs in this range which were spent within the next given blocks (exclusive)");
    m.def("outputs_spent_outside_relative_height",
          [](Range &range, int difference) -> ranges::any_view<blocksci::Output> {
              return outputsSpentOutsideRelativeHeight(range, difference);
          }, "Returns a list of all of the outputs in this range which were spent outside the next given blocks (inclusive)");
    m.def("output_value", [](Range &range) { return outputValue(range); }, "Return the total output value contained");
    
    cl.def_property_readonly("outs",  [](Range &chain) -> ranges::any_view<blocksci::Output> {
        return outputs(chain);
    })
    ;
}

template<typename Class>
void addInputAlgorithms(pybind11::module &m, Class & cl) {
    using Range = typename Class::type;
    m.def("input_value", [](Range &range) { return inputValue(range); }, "Return the total input value contained");
    
    m.def("inputs_created_before_height",
          [](Range &range, uint32_t blockHeight) -> ranges::any_view<blocksci::Input> {
              return inputsCreatedBeforeHeight(range, blockHeight);
          }, "Returns a list of all of the inputs in this range which spent outputs before the given height (exclusive)");
    
    m.def("inputs_created_after_height",
          [](Range &range, uint32_t blockHeight) -> ranges::any_view<blocksci::Input> {
              return inputsCreatedAfterHeight(range, blockHeight);
          }, "Returns a list of all of the inputs in this range which spent outputs after the given height (inclusive)");
    
    m.def("inputs_created_within_relative_height",
          [](Range &range, int difference) -> ranges::any_view<blocksci::Input> {
              return inputsCreatedWithinRelativeHeight(range, difference);
          }, "Returns a list of all of the inputs in this range which spent outputs before the given relative height (exclusive)");
    
    m.def("inputs_created_outside_relative_height",
          [](Range &range, int difference) -> ranges::any_view<blocksci::Input> {
              return inputsCreatedOutsideRelativeHeight(range, difference);
          }, "Returns a list of all of the inputs in this range which spent outputs after the given relative height (inclusive)");
    
    cl.def_property_readonly("ins",  [](Range &chain) -> ranges::any_view<blocksci::Input> {
        return inputs(chain);
    })
    ;
}

template<typename Class>
void addTxAlgorithms(pybind11::module &m, Class & cl) {
    using Range = typename Class::type;
    addOutputAlgorithms(m, cl);
    addInputAlgorithms(m, cl);
    
    m.def("input_count", [](Range &range) { return inputCount(range); }, "Returns the number of inputs contained");
    
    m.def("output_count", [](Range &range) { return outputCount(range); }, "Returns the number of outputs contained");
    
    m.def("fee", [](Range &range) { return totalFee(range); }, "Returns the total amount of fee contained");
}

template<typename Class>
void addTxRangeAlgorithms(pybind11::module &m, Class & cl) {
    using Range = typename Class::type;
    addTxAlgorithms(m, cl);
    
    m.def("fees", [](Range &range) { return fees(range) | ranges::to_vector; }, "Returns a list of all of the outputs in this range that are still unspent");
    
    m.def("fees_per_byte", [](Range &range){ return feesPerByte(range) | ranges::to_vector; }, "Returns a list of all of the outputs in this range that are still unspent");
}

template<typename Class>
void addBlockRangeAlgorithms(pybind11::module &m, Class & cl) {
    using Range = typename Class::type;
    addTxRangeAlgorithms(m, cl);
    
    cl.def_property_readonly("txes",  [](Range &chain) -> ranges::any_view<blocksci::Transaction> {
        return txes(chain);
    });
}


template<typename Class, std::enable_if_t<std::is_same<ranges::range_value_type_t<typename Class::type>, blocksci::Block>::value, int> = 0>
void addChainAlgorithms(pybind11::module &m, Class & cl) {
  addBlockRangeAlgorithms(m, cl);
}

template<typename Class, std::enable_if_t<std::is_same<ranges::range_value_type_t<typename Class::type>, blocksci::Transaction>::value, int> = 0>
void addChainAlgorithms(pybind11::module &m, Class & cl) {
  addTxRangeAlgorithms(m, cl);
}

template<typename Class, std::enable_if_t<std::is_same<typename Class::type, blocksci::Transaction>::value, int> = 0>
void addChainAlgorithms(pybind11::module &m, Class & cl) {
  addTxAlgorithms(m, cl);
}

template<typename Class, std::enable_if_t<std::is_same<ranges::range_value_type_t<typename Class::type>, blocksci::Input>::value, int> = 0>
void addChainAlgorithms(pybind11::module &m, Class & cl) {
  addInputAlgorithms(m, cl);
}

template<typename Class, std::enable_if_t<std::is_same<ranges::range_value_type_t<typename Class::type>, blocksci::Output>::value, int> = 0>
void addChainAlgorithms(pybind11::module &m, Class & cl) {
  addOutputAlgorithms(m, cl);
}

#endif /* python_chain_algorithms_hpp */
