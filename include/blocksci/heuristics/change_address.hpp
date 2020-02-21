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
#include <range/v3/iterator/operations.hpp>
#include <range/v3/view.hpp>
#include <range/v3/view/set_algorithm.hpp>

#include <unordered_set>

#define CHANGE_ADDRESS_TYPE_LIST VAL(PeelingChain), VAL(PowerOfTen), VAL(OptimalChange), VAL(AddressType), VAL(Locktime), VAL(AddressReuse), VAL(ClientChangeAddressBehavior), VAL(Legacy), VAL(FixedFee), VAL(None), VAL(Spent)
#define CHANGE_ADDRESS_TYPE_SET VAL(PeelingChain), VAL(PowerOfTen), VAL(OptimalChange), VAL(AddressType) VAL(Locktime), VAL(AddressReuse), VAL(ClientChangeAddressBehavior), VAL(Legacy), VAL(FixedFee), VAL(None), VAL(Spent)
namespace blocksci {
namespace heuristics {
    
    bool BLOCKSCI_EXPORT isPeelingChain(const Transaction &tx);
    
    struct BLOCKSCI_EXPORT ChangeType {
        enum Enum {
        #define VAL(x) x
            CHANGE_ADDRESS_TYPE_LIST
        #undef VAL
        };
        #define VAL(x) Enum::x
        static constexpr std::array<Enum, 11> all = {{CHANGE_ADDRESS_TYPE_LIST}};
        #undef VAL
        static constexpr size_t size = all.size();
    };
    
    template <ChangeType::Enum heuristic>
    struct BLOCKSCI_EXPORT ChangeHeuristicImpl {
        ranges::any_view<Output> operator()(const Transaction &tx) const;
    };
    
    template<>
    struct BLOCKSCI_EXPORT ChangeHeuristicImpl<ChangeType::PowerOfTen> {
        int digits;
        ChangeHeuristicImpl(int digits_ = 6) : digits(digits_) {}
        ranges::any_view<Output> operator()(const Transaction &tx) const;
    };
    
    using PeelingChainChange = ChangeHeuristicImpl<ChangeType::PeelingChain>;
    using PowerOfTenChange = ChangeHeuristicImpl<ChangeType::PowerOfTen>;
    using OptimalChangeChange = ChangeHeuristicImpl<ChangeType::OptimalChange>;
    using AddressTypeChange = ChangeHeuristicImpl<ChangeType::AddressType>;
    using LocktimeChange = ChangeHeuristicImpl<ChangeType::Locktime>;
    using AddressReuseChange = ChangeHeuristicImpl<ChangeType::AddressReuse>;
    using ClientChangeAddressBehaviorChange = ChangeHeuristicImpl<ChangeType::ClientChangeAddressBehavior>;
    using LegacyChange = ChangeHeuristicImpl<ChangeType::Legacy>;
    using FixedFee = ChangeHeuristicImpl<ChangeType::FixedFee>;
    using NoChange = ChangeHeuristicImpl<ChangeType::None>;
    using Spent = ChangeHeuristicImpl<ChangeType::Spent>;
    
    struct BLOCKSCI_EXPORT ChangeHeuristic {
        using HeuristicFunc = std::function<ranges::any_view<Output>(const Transaction &tx)>;
        
        HeuristicFunc impl;
        
        ChangeHeuristic(HeuristicFunc func) : impl(std::move(func)) {}
        
        template<typename T>
        ChangeHeuristic(T func) : impl(std::move(func)) {}
        
        ranges::any_view<Output> operator()(const Transaction &tx) const {
            return impl(tx);
        }
        
        static ChangeHeuristic uniqueChange(ChangeHeuristic ch) {
            return ChangeHeuristic{HeuristicFunc{[=](const Transaction &tx) {
                auto c = ch(tx);
                if (ranges::distance(c) == 1) {
                    return c;
                } else {
                    ranges::any_view<Output> empty = ranges::view::empty<Output>;
                    return empty;
                }
            }}};
        }
        
        static ChangeHeuristic setIntersection(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[=](const Transaction &tx) {
                auto first = a(tx);
                auto second = b(tx);
                return ranges::view::set_intersection(first, second);
            }}};
        }
        
        static ChangeHeuristic setUnion(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[=](const Transaction &tx) {
                auto first = a(tx);
                auto second = b(tx);
                return ranges::view::set_union(first, second);
            }}};
        }
        
        static ChangeHeuristic setDifference(ChangeHeuristic a, ChangeHeuristic b) {
            return ChangeHeuristic{HeuristicFunc{[=](const Transaction &tx) {
                auto first = a(tx);
                auto second = b(tx);
                return ranges::view::set_difference(first, second);
            }}};
        }
    };
}  // namespace heuristics
}  // namespace blocksci

#endif /* change_address_hpp */
