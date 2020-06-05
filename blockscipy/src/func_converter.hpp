//
//  func_converter.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef function_converter_h
#define function_converter_h

#include <functional>

template <typename Derived, typename Return, typename Class, typename... Args>
auto func_adaptor(Return (Class::*pmf)(Args...)) -> std::function<Return(Derived &, Args...)> { return pmf; }

template <typename Derived, typename Return, typename Class, typename... Args>
auto func_adaptor(Return (Class::*pmf)(Args...) const) -> std::function<Return(Derived &, Args...)> { return pmf; }

template <typename Derived, typename Return, typename First, typename... Args>
auto func_adaptor(Return (*pmf)(First, Args...)) -> std::function<Return(Derived &, Args...)> { return pmf; }

#endif // function_converter_h