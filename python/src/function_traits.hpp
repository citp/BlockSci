//
//  function_traits.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef function_traits_h
#define function_traits_h

#include <tuple>

template <typename T, bool = true>
struct function_traits {
private:
	using member_func = function_traits<decltype(&T::operator()), false>;
public:
	using result_type = typename member_func::result_type;
    using arg_tuple = typename member_func::arg_tuple;
    static constexpr auto arity = member_func::arity;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const, false> : public function_traits<ReturnType(Args...)> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const, true> : public function_traits<ReturnType(const ClassType&, Args...)> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...), false> : public function_traits<ReturnType(Args...)> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...), true> : public function_traits<ReturnType(ClassType&, Args...)> {};


template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(*)(Args...)> : public function_traits<ReturnType(Args...)> {};

template <typename ReturnType, typename... Args>
struct function_traits<ReturnType(Args...)>
// we specialize for pointers to member function
{
    using result_type = ReturnType;
    using arg_tuple = std::tuple<Args...>;
    static constexpr auto arity = sizeof...(Args);
};

#endif /* function_traits_h */
