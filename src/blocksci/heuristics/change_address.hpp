//
//  change_address.hpp
//  blocksci
//
//  Created by Malte Möser on 10/2/17.
//

#ifndef change_address_hpp
#define change_address_hpp

#include <blocksci/blocksci_export.h>
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
    
    struct BLOCKSCI_EXPORT ChangeType {
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
    struct BLOCKSCI_EXPORT ChangeHeuristicImpl : ChangeHeuristicImplParent<ChangeHeuristicImpl<heuristic>> {
        std::unordered_set<Output> operator()(const Transaction &tx) const;
    };
    
    template<>
    struct BLOCKSCI_EXPORT ChangeHeuristicImpl<ChangeType::PowerOfTen> : ChangeHeuristicImplParent<ChangeHeuristicImpl<ChangeType::PowerOfTen>> {
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
        
    struct BLOCKSCI_EXPORT ChangeHeuristic {
        
        using HeuristicFunc = std::function<std::unordered_set<Output>(const Transaction &tx)>;
        
        HeuristicFunc impl;
        
        ChangeHeuristic(HeuristicFunc func) : impl(std::move(func)) {}
        
        template<typename T>
        ChangeHeuristic(T func) : impl(std::move(func)) {}
        
        std::unordered_set<Output> operator()(const Transaction &tx) const {
            return impl(tx);
        }
        
        ranges::optional<Output> uniqueChange(const Transaction &tx) const {
            return internal::singleOrNullptr(impl(tx));
        }
        
        static ChangeHeuristic setIntersection(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[=](const Transaction &tx) {
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
        
        static ChangeHeuristic setUnion(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[=](const Transaction &tx) {
                auto first = a(tx);
                auto second = b(tx);
                for (auto &item : second) {
                    first.insert(item);
                }
                return first;
            }}};
        }
        
        static ChangeHeuristic setDifference(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[=](const Transaction &tx) {
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
    std::unordered_set<Output> BLOCKSCI_EXPORT changeByPeelingChain(const Transaction &tx);
    ranges::optional<Output> BLOCKSCI_EXPORT uniqueChangeByPeelingChain(const Transaction &tx);
    
    // Detects a change output by checking for output values that are multiples of 10^digits.
    std::unordered_set<Output> BLOCKSCI_EXPORT changeByPowerOfTenValue(const Transaction &tx, int digits = 6);
    ranges::optional<Output> BLOCKSCI_EXPORT uniqueChangeByPowerOfTenValue(const Transaction &tx, int digits=6);
    
    std::unordered_set<Output> BLOCKSCI_EXPORT changeByOptimalChange(const Transaction &tx);
    ranges::optional<Output> BLOCKSCI_EXPORT uniqueChangeByOptimalChange(const Transaction &tx);
    
    std::unordered_set<Output> BLOCKSCI_EXPORT changeByAddressType(const Transaction &tx);
    ranges::optional<Output> BLOCKSCI_EXPORT uniqueChangeByAddressType(const Transaction &tx);
    
    std::unordered_set<Output> BLOCKSCI_EXPORT changeByLocktime(const Transaction &tx);
    ranges::optional<Output> BLOCKSCI_EXPORT uniqueChangeByLocktime(const Transaction &tx);
    
    std::unordered_set<Output> BLOCKSCI_EXPORT changeByAddressReuse(const Transaction &tx);
    ranges::optional<Output> BLOCKSCI_EXPORT uniqueChangeByAddressReuse(const Transaction &tx);
    
    std::unordered_set<Output> BLOCKSCI_EXPORT changeByClientChangeAddressBehavior(const Transaction &tx);
    ranges::optional<Output> BLOCKSCI_EXPORT uniqueChangeByClientChangeAddressBehavior(const Transaction &tx);
    
    std::unordered_set<Output> BLOCKSCI_EXPORT changeByLegacyHeuristic(const Transaction &tx);
    ranges::optional<Output> BLOCKSCI_EXPORT uniqueChangeByLegacyHeuristic(const Transaction &tx);
}}

#endif /* change_address_hpp */
