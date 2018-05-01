//
//  func_converter.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef function_converter_h
#define function_converter_h

#include <functional>

template <typename T, bool = true>
struct to_func_impl {
private:
	using member_func = to_func_impl<decltype(&T::operator()), false>;
public:
	using type = typename member_func::type;
};

template <typename ClassType, typename ReturnType, typename... Args>
struct to_func_impl<ReturnType(ClassType::*)(Args...) const, false> : public to_func_impl<ReturnType(Args...)> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct to_func_impl<ReturnType(ClassType::*)(Args...) const, true> : public to_func_impl<ReturnType(const ClassType&, Args...)> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct to_func_impl<ReturnType(ClassType::*)(Args...), false> : public to_func_impl<ReturnType(Args...)> {};

template <typename ClassType, typename ReturnType, typename... Args>
struct to_func_impl<ReturnType(ClassType::*)(Args...), true> : public to_func_impl<ReturnType(ClassType&, Args...)> {};


template <typename ReturnType, typename... Args>
struct to_func_impl<ReturnType(*)(Args...)> : public to_func_impl<ReturnType(Args...)> {};

template <typename ReturnType, typename... Args>
struct to_func_impl<ReturnType(Args...)>
// we specialize for pointers to member function
{
	using type = std::function<ReturnType(Args...)>;
};


template <typename F>
typename to_func_impl<F>::type toFunc(F && f) {
	return {std::forward<F>(f)};
}

#endif // function_converter_h