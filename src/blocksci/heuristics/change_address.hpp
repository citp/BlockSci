//
//  change_address.hpp
//  blocksci
//
//  Created by Malte Möser on 10/2/17.
//

#ifndef change_address_hpp
#define change_address_hpp

#include <blocksci/chain/chain_fwd.hpp>
#include <blocksci/chain/output.hpp>
#include <blocksci/scripts/scripts_fwd.hpp>

#include <range/v3/utility/optional.hpp>

#include <unordered_set>

#define CHANGE_ADDRESS_TYPE_LIST VAL(PeelingChain), VAL(PowerOfTen), VAL(OptimalChange), VAL(AddressType), VAL(Locktime), VAL(AddressReuse), VAL(ClientChangeAddressBehavior), VAL(Legacy)
#define CHANGE_ADDRESS_TYPE_SET VAL(PeelingChain) VAL(PowerOfTen) VAL(OptimalChange) VAL(AddressType) VAL(Locktime), VAL(AddressReuse), VAL(ClientChangeAddressBehavior) VAL(Legacy)
namespace blocksci {
namespace heuristics {
    
    namespace internal {
        ranges::optional<Output> singleOrNullptr(std::unordered_set<Output> candidates);
    } // namespace internal
    
    bool isPeelingChain(const Transaction &tx);
    
    struct ChangeType {
        enum Enum {
        #define VAL(x) x
            CHANGE_ADDRESS_TYPE_LIST
        #undef VAL
        };
        #define VAL(x) Enum::x
        static constexpr std::array<Enum, 8> all = {{CHANGE_ADDRESS_TYPE_LIST}};
        #undef VAL
        static constexpr size_t size = all.size();
    };
    
    template <typename T>
    struct ChangeHeuristicImplParent {
        ranges::optional<Output> uniqueChange(const Transaction &tx) const {
            return singleOrNullptr((*static_cast<const T *>(this))(tx));
        }
    };
    
    template <ChangeType::Enum heuristic>
    struct ChangeHeuristicImpl : ChangeHeuristicImplParent<ChangeHeuristicImpl<heuristic>> {
        std::unordered_set<Output> operator()(const Transaction &tx) const;
    };
    
    template<>
    struct ChangeHeuristicImpl<ChangeType::PowerOfTen> : ChangeHeuristicImplParent<ChangeHeuristicImpl<ChangeType::PowerOfTen>> {
        int digits;
        ChangeHeuristicImpl(int digits_ = 5) : ChangeHeuristicImplParent{}, digits(digits_) {}
        std::unordered_set<Output> operator()(const Transaction &tx) const;
    };
    
    using PeelingChainChange = ChangeHeuristicImpl<ChangeType::PeelingChain>;
    using PowerOfTenChange = ChangeHeuristicImpl<ChangeType::PowerOfTen>;
    using OptimalChangeChange = ChangeHeuristicImpl<ChangeType::OptimalChange>;
    using AddressTypeChange = ChangeHeuristicImpl<ChangeType::AddressType>;
    using LocktimeChange = ChangeHeuristicImpl<ChangeType::Locktime>;
    using AddressReuseChange = ChangeHeuristicImpl<ChangeType::AddressReuse>;
    using ClientChangeAddressBehaviorChange = ChangeHeuristicImpl<ChangeType::ClientChangeAddressBehavior>;
    using LegacyChange = ChangeHeuristicImpl<ChangeType::Legacy>;
        
    struct ChangeHeuristic {
        
        using HeuristicFunc = std::function<std::unordered_set<Output>(const Transaction &tx)>;
        
        HeuristicFunc impl;
        
        ChangeHeuristic(HeuristicFunc func) : impl(std::move(func)) {}
        
        template<typename T>
        ChangeHeuristic(T func) : impl(std::move(func)) {}
        
        std::unordered_set<Output> operator()(const Transaction &tx) {
            return impl(tx);
        }
        
        ranges::optional<Output> uniqueChange(const Transaction &tx) const {
            return internal::singleOrNullptr(impl(tx));
        }
        
        static ChangeHeuristic heuristicIntersection(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[&](const Transaction &tx) {
                auto first = a(tx);
                auto second = b(tx);
                for(auto it = begin(first); it != end(first);) {
                    if (second.find(*it) == end(second)){
                        it = first.erase(it);
                    } else {
                        ++it;
                    }
                }
                return first;
            }}};
        }
        
        static ChangeHeuristic heuristicUnion(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[&](const Transaction &tx) {
                auto first = a(tx);
                auto second = b(tx);
                for (auto &item : second) {
                    first.insert(item);
                }
                return first;
            }}};
        }
        
        static ChangeHeuristic heuristicDifference(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[&](const Transaction &tx) {
                auto first = a(tx);
                auto second = b(tx);
                for(auto it = begin(first); it != end(first);) {
                    if (second.find(*it) != end(second)){
                        it = first.erase(it);
                    } else {
                        ++it;
                    }
                }
                return first;
            }}};
        }
    };
    
    // If tx is a peeling chain, returns the smaller output.
    std::unordered_set<Output> changeByPeelingChain(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByPeelingChain(const Transaction &tx);
    
    // Detects a change output by checking for output values that are multiples of 10^digits.
    std::unordered_set<Output> changeByPowerOfTenValue(const Transaction &tx, int digits = 6);
    ranges::optional<Output> uniqueChangeByPowerOfTenValue(const Transaction &tx, int digits=6);
    
    std::unordered_set<Output> changeByOptimalChange(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByOptimalChange(const Transaction &tx);
    
    std::unordered_set<Output> changeByAddressType(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByAddressType(const Transaction &tx);
    
    std::unordered_set<Output> changeByLocktime(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByLocktime(const Transaction &tx);
    
    std::unordered_set<Output> changeByAddressReuse(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByAddressReuse(const Transaction &tx);
    
    std::unordered_set<Output> changeByClientChangeAddressBehavior(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByClientChangeAddressBehavior(const Transaction &tx);
    
    std::unordered_set<Output> changeByLegacyHeuristic(const Transaction &tx);
    ranges::optional<Output> uniqueChangeByLegacyHeuristic(const Transaction &tx);
}}

#endif /* change_address_hpp */
