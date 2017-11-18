//
//  algorithms.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/17.
//

#ifndef chain_algorithms_hpp
#define chain_algorithms_hpp

#include <blocksci/address/address_types.hpp>
#include <blocksci/scripts/script_type.hpp>

#include <range/v3/view/transform.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/numeric/accumulate.hpp>

namespace blocksci {
    
    template <typename T>
    inline auto inputs(const T &t) {
        return t | ranges::view::transform([&](const auto &a) { return inputs(a); }) | ranges::view::join;
    }
    
    template <typename T>
    inline auto outputs(const T &t) {
        return t | ranges::view::transform([&](const auto &a) { return outputs(a); }) | ranges::view::join;
    }
    
    template <typename T>
    inline auto outputsUnspent(const T &t) {
        return t | ranges::view::transform([&](const auto &a) { return outputsUnspent(a); }) | ranges::view::join;
    }
    
    template <typename T>
    inline auto outputsOfAddressType(const T &t, AddressType::Enum type) {
        return t | ranges::view::transform([&](const auto &a) { return outputsOfAddressType(a, type); }) | ranges::view::join;
    }
    
    template <typename T>
    inline auto outputsOfScriptType(const T &t, ScriptType::Enum type) {
        return t | ranges::view::transform([&](const auto &a) { return outputsOfScriptType(a, type); }) | ranges::view::join;
    }
    
    template <typename T>
    inline uint64_t inputCount(const T &t) {
        auto values =  t | ranges::view::transform([&](const auto &a) { return inputCount(a); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t outputCount(const T &t) {
        auto values =  t | ranges::view::transform([&](const auto &a) { return outputCount(a); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t totalOut(const T &t) {
        auto values = t | ranges::view::transform([](const auto &a) { return totalOut(a); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t totalIn(const T &t) {
        auto values = t | ranges::view::transform([](const auto &a) { return totalIn(a); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename T>
    inline uint64_t fee(const T &t) {
        auto values = t | ranges::view::transform([](const auto &a) { return fee(a); });
        return ranges::accumulate(values, uint64_t{0});
    }
    
    template <typename B>
    inline auto fees(const B &b) {
        return b | ranges::view::transform([&](const auto &a) { return fees(a); }) | ranges::view::join;
    }
    
    template <typename B>
    inline auto feesPerByte(const B &b) {
        return b | ranges::view::transform([&](const auto &a) { return feesPerByte(a); }) | ranges::view::join;
    }
}

#endif /* chain_algorithms_hpp */
