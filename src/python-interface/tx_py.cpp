//
//  tx_py.cpp
//  blocksci
//
//  Created by Harry Kalodner on 7/4/17.
//
//

#include "optional_py.hpp"
#include "ranges_py.hpp"

#include <blocksci/chain/algorithms.hpp>
#include <blocksci/chain/blockchain.hpp>
#include <blocksci/chain/transaction.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/chain/input.hpp>
#include <blocksci/chain/block.hpp>
#include <blocksci/address/address.hpp>
#include <blocksci/util/bitcoin_uint256.hpp>
#include <blocksci/heuristics/tx_identification.hpp>
#include <blocksci/heuristics/change_address.hpp>
#include <blocksci/index/address_index.hpp>
#include <blocksci/index/hash_index.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <pybind11/chrono.h>

#include <range/v3/iterator_range.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>

namespace py = pybind11;

using namespace blocksci;

template <typename Class, typename FuncApplication, typename FuncDoc>
void addTransactionMethods(Class &cl, FuncApplication func, FuncDoc func2) {
    cl
    .def_property_readonly("output_count", func([](const Transaction &tx) {
        return tx.outputCount();
    }), func2("The number of outputs this transaction has"))
    .def_property_readonly("input_count", func([](const Transaction &tx) {
        return tx.inputCount();
    }), func2("The number of inputs this transaction has"))
    .def_property_readonly("size_bytes", func([](const Transaction &tx) {
        return tx.totalSize();
    }), func2("The size of this transaction in bytes"))
    .def_property_readonly("base_size", func([](const Transaction &tx) {
        return tx.baseSize();
    }), func2("The size of the non-segwit data in bytes"))
    .def_property_readonly("total_size", func([](const Transaction &tx) {
        return tx.totalSize();
    }), func2("The size all transaction data in bytes"))
    .def_property_readonly("virtual_size", func([](const Transaction &tx) {
        return tx.virtualSize();
    }), func2("The weight of the transaction divided by 4"))
    .def_property_readonly("weight", func([](const Transaction &tx) {
        return tx.weight();
    }), func2("Three times the base size plus the total size"))
    .def_property_readonly("locktime", func([](const Transaction &tx) {
        return tx.locktime();
    }), func2("The locktime of this transasction"))
    .def_property_readonly("block_height", func([](const Transaction &tx) {
        return tx.blockHeight;
    }), func2("The height of the block that this transaction was in"))
    .def_property_readonly("block_time", func([](const Transaction &tx) {
        return tx.block().getTime();
    }), func2("The time that the block containing this transaction arrived"))
    .def_property_readonly("block", func([](const Transaction &tx) {
        return tx.block();
    }), func2("The block that this transaction was in"))
    .def_property_readonly("index", func([](const Transaction &tx) {
        return tx.txNum;
    }), func2("The internal index of this transaction"))
    .def_property_readonly("hash", func([](const Transaction &tx) {
        return tx.getHash();
    }), func2("The 256-bit hash of this transaction"))
    .def_property_readonly("input_value", func([](const Transaction &tx) {
        return totalInputValue(tx);
    }), func2("The sum of the value of all of the inputs"))
    .def_property_readonly("output_value", func([](const Transaction &tx) {
        return totalOutputValue(tx);
    }), func2("The sum of the value of all of the outputs"))
    .def_property_readonly("output_value", func([](const Transaction &tx) {
        return totalOutputValue(tx);
    }), func2("The sum of the value of all of the outputs"))
    .def_property_readonly("fee", func([](const Transaction &tx) {
        return fee(tx);
    }), func2("The fee paid by this transaction"))
    .def("fee_per_byte", func([](const Transaction &tx, const std::string &sizeMeasure) {
        auto txFee = fee(tx);
        if (sizeMeasure == "total") {
            return txFee / tx.totalSize();
        } else if (sizeMeasure == "base") {
            return txFee / tx.baseSize();
        } else if(sizeMeasure == "weight") {
            return txFee / tx.weight();
        } else if(sizeMeasure == "virtual") {
            return txFee / tx.virtualSize();
        } else {
            throw std::invalid_argument{"Size measure must be one of total, base, weight, or virtual"};
        }
    }), py::arg("size_measure") = "virtual", func2("The ratio of fee paid to size of this transaction. By default this uses veritual size, but passing total, base, weight, or virtual let's you choose what measure of size you want"))
    .def_property_readonly("op_return", func([](const Transaction &tx) {
        return getOpReturn(tx);
    }), func2("If this transaction included a null data address, return its output. Otherwise return None"))
    .def_property_readonly("is_coinbase", func([](const Transaction &tx) {
        return tx.isCoinbase();
    }), func2("Return's true if this transaction is a Coinbase transaction"))
    .def_property_readonly("change_output", func([](const Transaction &tx) {
        return heuristics::uniqueChangeByLegacyHeuristic(tx);
    }), func2("If the change address in this transaction can be determined via the fresh address criteria, return it. Otherwise return None."))
    ;
}

template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())>
{};

template <typename ClassType, typename ReturnType, typename First, typename... Args>
struct function_traits<ReturnType(ClassType::*)(First, Args...) const>
// we specialize for pointers to member function
{
    using result_type = ReturnType;
    using arg_tuple = std::tuple<Args...>;
    static constexpr auto arity = sizeof...(Args);
};

template <class F, std::size_t ... Is, class T>
auto lambda_to_func_impl(F f, std::index_sequence<Is...>, T) {
    return std::function<typename T::result_type(std::tuple_element_t<Is, typename T::arg_tuple>...)>(f);
}

template <class F>
auto lambda_to_func(F f) {
    using traits = function_traits<F>;
    return lambda_to_func_impl(f, std::make_index_sequence<traits::arity>{}, traits{});
}

//template <class F, std::size_t ... Is, class T>
//auto applyTxMethodsToTxRangeImpl(F f, std::index_sequence<Is...>, T) {
//    return [&f](ranges::any_view<Transaction> &view, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
//        std::vector<typename T::result_type> ret;
//        RANGES_FOR(auto && tx, view) {
//            ret.push_back(f(std::forward<decltype(tx)>(tx), args...));
//        }
//        return py::array(ret.size(), ret.data());
//    };
//}
//
//template <typename F>
//auto applyTxMethodsToTxRange(F f) {
//    using traits = function_traits<F>;
//    return applyTxMethodsToTxRangeImpl(f, std::make_index_sequence<traits::arity>{}, traits{});
//}

template <class F, std::size_t ... Is, class T>
auto applyTxMethodsToTxRangeImpl(F f, std::index_sequence<Is...>, T, std::true_type) {
    return [&f](ranges::any_view<Transaction> &view, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        std::vector<typename T::result_type> ret;
        RANGES_FOR(auto && tx, view) {
            ret.push_back(f(std::forward<decltype(tx)>(tx), args...));
        }
        return py::array(ret.size(), ret.data());
    };
}

template <class F, std::size_t ... Is, class T>
auto applyTxMethodsToTxRangeImpl(F f, std::index_sequence<Is...>, T, std::false_type) {
    return [&f](ranges::any_view<Transaction> &view, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        py::list list;
        RANGES_FOR(auto && tx, view) {
            list.append(f(std::forward<decltype(tx)>(tx), args...));
        }
        return list;
    };
}

template<typename... Conds>
struct and_
: std::true_type
{ };

template<typename Cond, typename... Conds>
struct and_<Cond, Conds...>
: std::conditional<Cond::value, and_<Conds...>, std::false_type>::type
{ };

template <typename F>
auto applyTxMethodsToTxRange(F f) {
    using traits = function_traits<F>;
    return applyTxMethodsToTxRangeImpl(f, std::make_index_sequence<traits::arity>{}, traits{}, and_<py::detail::satisfies_any_of<typename traits::result_type, std::is_arithmetic, py::detail::is_complex>, py::detail::vector_has_data_and_format<std::vector<typename traits::result_type>>>{});
}

//template <typename F>
//auto applyTxMethodsToTxRange(F f) {
//    using traits = function_traits<F>;
//    return applyTxMethodsToTxRangeImpl(f, std::make_index_sequence<traits::arity>{}, traits{}, py::detail::satisfies_any_of<typename traits::result_type, std::is_arithmetic, py::detail::is_complex>{});
//}

//template <typename F>
//auto applyTxMethodsToTxRange(F f) {
//    using traits = function_traits<F>;
//    return applyTxMethodsToTxRangeImpl(f, std::make_index_sequence<traits::arity>{}, traits{}, py::detail::is_pod_struct<typename traits::result_type>{});
//}



template <class F, std::size_t ... Is, class T>
auto applyTxMethodsToTxImpl(F f, std::index_sequence<Is...>, T) {
    return [&f](const Transaction &tx, const std::tuple_element_t<Is, typename T::arg_tuple> &... args) {
        return f(std::forward<decltype(tx)>(tx), args...);
    };
}

template <typename F>
auto applyTxMethodsToTx(F f) {
    using traits = function_traits<F>;
    return applyTxMethodsToTxImpl(f, std::make_index_sequence<traits::arity>{}, traits{});
}


void init_tx(py::module &m) {
    
//    template <typename Ret, typename... Args>
//    auto applyTxMethodsToTx(std::function<Ret(const Transaction &tx, Args...)>) {
//        return [](const Transaction &tx, Args && ...args) {
//            return func(tx, std::forward<Args>(args)...);
//        }
//    }

    
    auto txRangeClass = addRangeClass<ranges::any_view<Transaction>>(m, "AnyTxRange");
    addTransactionMethods(txRangeClass, [](auto func) {
        return applyTxMethodsToTxRange(func);
    }, [](std::string docstring) {
        std::stringstream ss;
        ss << "For each transaction: " << docstring;
        return strdup(ss.str().c_str());
    });

    txRangeClass
    .def_property_readonly("inputs", [](ranges::any_view<Transaction> &txes) -> ranges::any_view<ranges::any_view<Input>>  {
        return txes | ranges::view::transform([](const Transaction &tx) -> ranges::any_view<Input> {
            return tx.inputs();
        });
    }, "A list of lists of the inputs of each transaction")
    .def_property_readonly("outputs", [](ranges::any_view<Transaction> &txes) -> ranges::any_view<ranges::any_view<Output>>  {
        return txes | ranges::view::transform([](const Transaction &tx) -> ranges::any_view<Output> {
            return tx.outputs();
        });
    }, "A list of lists of the outputs of each transaction")
    ;
    
    
    py::class_<Transaction> txClass(m, "Tx", "Class representing a transaction in a block");
    addTransactionMethods(txClass, [](auto func) {
        return applyTxMethodsToTx(func);
    }, [](auto && docstring) {
        return std::forward<decltype(docstring)>(docstring);
    });
    
    txClass
    .def("__str__", &Transaction::toString)
    .def("__repr__", &Transaction::toString)
    .def(py::self == py::self)
    .def(hash(py::self))
    .def_property_readonly("_access", &Transaction::getAccess)
    .def(py::init([](uint32_t index, const blocksci::Blockchain &chain) {
        return Transaction{index, chain.getAccess()};
    }), R"docstring(
         This functions gets the transaction with given index.
         
         :param int index: The index of the transation.
         :returns: Tx
         )docstring")
    .def(py::init([](const std::string hash, const blocksci::Blockchain &chain) {
        return Transaction{hash, chain.getAccess()};
    }), R"docstring(
         This functions gets the transaction with given hash.
         
         :param string index: The hash of the transation.
         :returns: Tx
         )docstring")
    .def_property_readonly("ins", [](const Transaction &tx) -> ranges::any_view<Input, ranges::get_categories<decltype(tx.inputs())>()>  {
        return tx.inputs();
    }, "A list of the inputs of the transaction") // same as below
    .def_property_readonly("inputs", [](const Transaction &tx) -> ranges::any_view<Input, ranges::get_categories<decltype(tx.inputs())>()>  {
        return tx.inputs();
    }, "A list of the inputs of the transaction") // same as above
    .def_property_readonly("outs", [](const Transaction &tx) -> ranges::any_view<Output, ranges::get_categories<decltype(tx.outputs())>()>  {
        return tx.outputs();
    }, "A list of the outputs of the transaction") // same as below
    .def_property_readonly("outputs", [](const Transaction &tx) -> ranges::any_view<Output, ranges::get_categories<decltype(tx.outputs())>()>  {
        return tx.outputs();
    }, "A list of the outputs of the transaction") // same as above
    ;
}
