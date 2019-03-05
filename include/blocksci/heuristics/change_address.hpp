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

#define CHANGE_ADDRESS_TYPE_LIST VAL(PeelingChain), VAL(PowerOfTen), VAL(OptimalChange), VAL(AddressType), VAL(Locktime), VAL(AddressReuse), VAL(ClientChangeAddressBehavior), VAL(Legacy), VAL(None), VAL(Spent)
#define CHANGE_ADDRESS_TYPE_SET VAL(PeelingChain), VAL(PowerOfTen), VAL(OptimalChange), VAL(AddressType) VAL(Locktime), VAL(AddressReuse), VAL(ClientChangeAddressBehavior), VAL(Legacy), VAL(None), VAL(Spent)
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
        static constexpr std::array<Enum, 10> all = {{CHANGE_ADDRESS_TYPE_LIST}};
        #undef VAL
        static constexpr size_t size = all.size();
    };
    
    template <ChangeType::Enum heuristic>
    struct BLOCKSCI_EXPORT ChangeHeuristicImpl {
        std::unordered_set<Output> operator()(const Transaction &tx) const;
        ranges::optional<Output> uniqueChange(const Transaction &tx) const;
    };
    
    template<>
    struct BLOCKSCI_EXPORT ChangeHeuristicImpl<ChangeType::PowerOfTen> {
        int digits;
        ChangeHeuristicImpl(int digits_ = 5) : digits(digits_) {}
        std::unordered_set<Output> operator()(const Transaction &tx) const;
        ranges::optional<Output> uniqueChange(const Transaction &tx) const;
    };
    
    using PeelingChainChange = ChangeHeuristicImpl<ChangeType::PeelingChain>;
    using PowerOfTenChange = ChangeHeuristicImpl<ChangeType::PowerOfTen>;
    using OptimalChangeChange = ChangeHeuristicImpl<ChangeType::OptimalChange>;
    using AddressTypeChange = ChangeHeuristicImpl<ChangeType::AddressType>;
    using LocktimeChange = ChangeHeuristicImpl<ChangeType::Locktime>;
    using AddressReuseChange = ChangeHeuristicImpl<ChangeType::AddressReuse>;
    using ClientChangeAddressBehaviorChange = ChangeHeuristicImpl<ChangeType::ClientChangeAddressBehavior>;
    using LegacyChange = ChangeHeuristicImpl<ChangeType::Legacy>;
    using NoChange = ChangeHeuristicImpl<ChangeType::None>;
    using Spent = ChangeHeuristicImpl<ChangeType::Spent>;
    
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
            auto candidates = impl(tx);
            if(candidates.size() == 1) {
                return *candidates.begin();
            } else {
                return ranges::nullopt;
            }
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
}}

#endif /* change_address_hpp */
