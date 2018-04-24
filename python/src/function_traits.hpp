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

#endif /* function_traits_h */
