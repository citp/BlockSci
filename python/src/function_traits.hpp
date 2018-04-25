//
//  function_traits.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef function_traits_h
#define function_traits_h

#include <tuple>

template <typename T>
struct function_traits {
private:
	using member_func = function_traits<decltype(&T::operator())>;
public:
	using result_type = typename member_func::result_type;
    using arg_tuple = typename member_func::arg_tuple;
    static constexpr auto arity = member_func::arity;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const> : public function_traits<ReturnType(Args...)> {};

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
